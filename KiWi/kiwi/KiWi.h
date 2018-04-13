#ifndef KiWi_h
#define KiWi_h

#include <atomic>
#include <memory>
#include <unordered_map>
#include <vector>
#include <list>
#include <set>
#include <iostream>

#include "../galios-utils/WorkListHelpers.h"
#include "ChunkIterator.h"
#include "Parameters.h"
#include "MultiChunkIterator.h"
#include "exceptionhelper.h"


using namespace std;

namespace kiwi
{
	template<typename K, typename V, class Comparer = less<K>>
	class KiWi : public ChunkIterator<K, V>
	{
    /*************** Constants ***************/
	public:
		static constexpr int MAX_THREADS = 32;
		static constexpr int PAD_SIZE = 640;
		
    /*************** Members ***************/
	private:
		LockFreeSkipListSet<Comparer, K, unique_ptr<Chunk<K, V, Comparer>>> skiplist; // skiplist of chunks for fast navigation
        const bool withScan; // support scan operations or not (scans add thread-array)
        vector<ScanData<K>*> scanArray; // TODO: Understand the proper pointer type choice
        Comparer compare;
        
	protected:
		atomic<int> version; // current version to add items with

    /*************** Constructors ***************/
	public:
		KiWi(const Chunk<K, V, Comparer>& head) : KiWi(head, false) {}

		KiWi(const Chunk<K, V, Comparer>& head, bool withScan) : withScan(withScan), scanArray(MAX_THREADS * (PAD_SIZE + 1))
		{
			version = AtomicInteger(2);        // first version is 2 - since 0 means NONE, and -1 means FREEZE
			skiplist.push(head.minKey, &head); // add first chunk (head) into skiplist

			if (withScan)
			{
				//this.threadArray = new ThreadData[MAX_THREADS];
			}
			else
			{
				//this.threadArray = null;
				this->scanArray.clear();
			}
		}

        /*************** Methods ***************/
        // <<<<<<<<<<<<<<<<<<<<<<<<
        
		static int pad(int idx)
		{
			return (PAD_SIZE + idx*PAD_SIZE);
		}

		virtual V get(K key)
		{
			// find chunk matching key
			Chunk<K, V, Comparer> *c = skiplist->floorEntry(key).getValue();
			c = iterateChunks(c, key);

			// help concurrent put operations (helpPut) set a version
			PutData<K, V> *pd = nullptr;
			pd = c->helpPutInGet(version->get(), key);

			// find item matching key inside chunk
			return c->find(key, pd);
		}

		virtual void put(K key, V val)
		{
			// find chunk matching key
			Chunk<K, V, Comparer> *c = skiplist->floorEntry(key).getValue();

			// repeat until put operation is successful
			while (true)
			{
				// the chunk we have might have been in part of split so not accurate
				// we need to iterate the chunks to find the correct chunk following it
				c = iterateChunks(c, key);

				// if chunk is infant chunk (has a parent), we can't add to it
				// we need to help finish compact for its parent first, then proceed
				{
					Chunk<K, V, Comparer> *parent = c->creator;
					if (parent != nullptr)
					{
						if (rebalance(parent) == nullptr)
						{
							return;
						}
					}
				}

				// allocate space in chunk for key & value
				// this also writes key&val into the allocated space
				int oi = c->allocate(key, val);

				// if failed - chunk is full, compact it & retry
				if (oi < 0)
				{
					c = rebalance(c);
					if (c == nullptr)
					{
						return;
					}
					continue;
				}

				if (withScan)
				{
					// publish put operation in thread array
					// publishing BEFORE setting the version so that other operations can see our value and help
					// this is in order to prevent us from adding an item with an older version that might be missed by others (scan/get)
					PutData<> tempVar(c, oi);
					c->publishPut(&tempVar);


					if (c->isFreezed())
					{
						// if succeeded to freeze item -- it is not accessible, need to reinsert it in rebalanced chunk
						if (c->tryFreezeItem(oi))
						{
							c->publishPut(nullptr);
							c = rebalance(c);

							continue;
						}
					}
				}

				// try to update the version to current version, but use whatever version is successfuly set
				// reading & setting version AFTER publishing ensures that anyone who sees this put op has
				// a version which is at least the version we're setting, or is otherwise setting the version itself
				int myVersion = c->setVersion(oi, this->version->get());

				// if chunk is frozen, clear published data, compact it and retry
				// (when freezing, version is set to FREEZE)
				if (myVersion == Chunk::FREEZE_VERSION)
				{
					// clear thread-array item if needed
					c->publishPut(nullptr);
					c = rebalance(c);
					continue;
				}

				// allocation is done (and published) and has a version
				// all that is left is to insert it into the chunk's linked list
				c->addToList(oi, key);

				// delete operation from thread array - and done
				c->publishPut(nullptr);

				if (shouldRebalance(c))
				{
					rebalance(c);
				}

				break;
			}
		}

	private:
		bool shouldRebalance(Chunk<K, V, Comparer> *c)
		{
			// perform actual check only in for pre defined percentage of puts
			if (ThreadLocalRandom::current().nextInt(100) > Parameters::rebalanceProbPerc)
			{
				return false;
			}

			// if another thread already runs rebalance -- skip it
			if (!c->isEngaged(nullptr))
			{
				return false;
			}
			int numOfItems = c->getNumOfItems();

			if ((c->sortedCount == 0 && numOfItems << 3 > Chunk::MAX_ITEMS) || (c->sortedCount > 0 && (c->sortedCount * Parameters::sortedRebalanceRatio) < numOfItems))
			{
				return true;
			}

			return false;
		}

	public:
		virtual void compactAllSerial()
		{
			Chunk<K, V, Comparer> *c = skiplist->firstEntry().getValue();
			while (c != nullptr)
			{
				c = rebalance(c);
				c = c->next.getReference();
			}

			c = skiplist->firstEntry().getValue();

			while (c != nullptr)
			{
				Chunk::ItemsIterator *iter = c->itemsIterator();
				Comparable prevKey = nullptr;
				int prevVersion = 0;

				if (iter->hasNext())
				{
					iter->next();
					prevKey = iter->getKey();
					prevVersion = iter->getVersion();
				}

				while (iter->hasNext())
				{
					iter->next();

					Comparable key = iter->getKey();
					int version = iter->getVersion();

					int cmp = prevKey(key);
					if (cmp >= 0)
					{
						throw IllegalStateException();
					}
					else if (cmp == 0)
					{
						if (prevVersion < version)
						{
							throw IllegalStateException();
						}
					}



					iter++;
				}

				c = c->next.getReference();
			}
			return;
		}

		virtual int scan(vector<V> &result, K min, K max)
		{
			// get current version and increment version (atomically) for this scan
			// all items beyond my version are ignored by this scan
			// the newVersion() method is used to ensure my version is published correctly,
			// so concurrent split ops will not compact items with this version (ensuring linearizability)
			int myVer = newVersion(min, max);


			// find chunk matching min key, to start iterator there
			Chunk<K, V, Comparer> *c = skiplist->floorEntry(min).getValue();
			c = iterateChunks(c, min);

			int itemsCount = 0;
			while (true)
			{

				if (c == nullptr || c->minKey->compareTo(max) > 0)
				{
					break;
				}

				// help pending put ops set a version - and get in a sorted map for use in the scan iterator
				// (so old put() op doesn't suddently set an old version this scan() needs to see,
				//  but after the scan() passed it)
				SortedMap<K, PutData<K, V>*> *items = c->helpPutInScan(myVer, min, max);

				itemsCount += c->copyValues(result, itemsCount, myVer, min, max, items);
				c = c->next.getReference();
			}

			// remove scan from scan array
			publishScan(nullptr);

			return itemsCount;
		}

		Chunk<K, V, Comparer> *getNext(Chunk<K, V, Comparer> *chunk) override
		{
			return chunk->next.getReference();
		}

		Chunk<K, V, Comparer> *getPrev(Chunk<K, V, Comparer> *chunk) override
		{
			return nullptr;
	/*
			Map.Entry<K, Chunk<K, V, Comparer>> kChunkEntry = skiplist.lowerEntry(chunk.minKey);
			if(kChunkEntry == null) return null;
			Chunk<Comparer, K,V> prev = kChunkEntry.getValue();
	
			while(true)
			{
				Chunk<Comparer, K,V> next = prev.next.getReference();
				if(next == chunk) break;
				if(next == null) {
					prev = null;
					break;
				}
	
				prev = next;
			}
	
			return prev;
	*/
		}

		/// <summary>
		/// fetch-and-add for the version counter. in a separate method because scan() ops need to use
		/// thread-array for this, to make sure concurrent split/compaction ops are aware of the scan() 
		/// </summary>
	private:
		int newVersion(K min, K max)
		{
			// create new ScanData and publish it - in it the scan's version will be stored
			ScanData *sd = new ScanData(min, max);
			publishScan(sd);

			// increment global version counter and get latest
			int myVer = version->getAndIncrement();

			// try to set it as this scan's version - return whatever is successfuly set
			if (sd->version.compareAndSet(Chunk::NONE, myVer))
			{
				return myVer;
			}
			else
			{
				return sd->version->get();
			}
		}

		/// <summary>
		/// finds and returns the chunk where key should be located, starting from given chunk </summary>
		Chunk<K, V, Comparer> *iterateChunks(Chunk<K, V, Comparer> *c, K key)
		{
			// find chunk following given chunk (next)
			Chunk<K, V, Comparer> *next = c->next.getReference();

			// found chunk might be in split process, so not accurate
			// since skiplist isn't updated atomically in split/compcation, our key might belong in the next chunk
			// next chunk might itself already be split, we need to iterate the chunks until we find the correct one
			while ((next != nullptr) && (next->minKey->compareTo(key) <= 0))
			{
				c = next;
				next = c->next.getReference();
			}

			return c;
		}

		vector<ScanData*> getScansArray(int myVersion)
		{

			vector<ScanData*> pScans(MAX_THREADS);
			bool isIncremented = false;
			int ver = -1;

			// read all pending scans
			for (int i = 0; i < MAX_THREADS; ++i)
			{
				ScanData *scan = scanArray[pad(i)];
				if (scan != nullptr)
				{
					pScans.push_back(scan);
				}
			}


			for (auto sd : pScans)
			{
				if (sd->version->get() == Chunk::NONE)
				{
					if (!isIncremented)
					{
						// increments version only once
						// if at least one pending scan has no version assigned
						ver = version->getAndIncrement();
						isIncremented = true;
					}

					sd->version.compareAndSet(Chunk::NONE,ver);
				}
			}

			return pScans;
		}

		set<Integer> getScans(int myVersion)
		{
			set<Integer> scans;

			// go over thread data of all threads
			for (int i = 0; i < MAX_THREADS; ++i)
			{
				// make sure data is for a Scan operation
				ScanData *currScan = scanArray[pad(i)];
				if (currScan == nullptr)
				{
					continue;
				}

				// if scan was published but didn't yet CAS its version - help it
				if (currScan->version->get() == Chunk::NONE)
				{
					// TODO: understand if we need to increment here
					int ver = version->getAndIncrement();
					currScan->version.compareAndSet(Chunk::NONE, ver);
				}

				// read the scan version (which is now set)
				int verScan = currScan->version->get();
				if (verScan < myVersion)
				{
					scans.insert(verScan);
				}
			}

			return scans;
		}

		Chunk<K, V, Comparer> *rebalance(Chunk<K, V, Comparer> *chunk)
		{
			Rebalancer<K, V> *rebalancer = new Rebalancer<K, V>(chunk, this);

			rebalancer = rebalancer->engageChunks();

			// freeze all the engaged range.
			// When completed, all update (put, next pointer update) operations on the engaged range
			// will be redirected to help the rebalance procedure
			rebalancer->freeze();

			vector<Chunk<K, V, Comparer>*> engaged = rebalancer->getEngagedChunks();
			// before starting compaction -- check if another thread has completed this stage
			if (!rebalancer->isCompacted())
			{
				ScanIndex<K> *index = updateAndGetPendingScans(version->get(), engaged);
				rebalancer->compact(index);
			}

			// the children list may be generated by another thread

			vector<Chunk<K, V, Comparer>*> compacted = rebalancer->getCompactedChunks();


			connectToChunkList(engaged, compacted);
			updateIndex(engaged, compacted);

			return compacted[0];
		}

		ScanIndex *updateAndGetPendingScans(int currVersion, vector<Chunk<K, V, Comparer>*> &engaged)
		{
			// TODO: implement versions selection by key
			K minKey = engaged[0]->minKey;
			Chunk<K, V, Comparer> *nextToRange = engaged[engaged.size() - 1]->next.getReference();
			K maxKey = nextToRange == nullptr ? nullptr : nextToRange->minKey;

			return new ScanIndex(getScansArray(currVersion), currVersion, minKey, maxKey);
		}

		void updateIndex(vector<Chunk<K, V, Comparer>*> &engagedChunks, vector<Chunk<K, V, Comparer>*> &compacted)
		{
			vector<Chunk<K, V, Comparer>*>::const_iterator iterEngaged = engagedChunks.begin();
			vector<Chunk<K, V, Comparer>*>::const_iterator iterCompacted = compacted.begin();

//JAVA TO C++ CONVERTER TODO TASK: Java iterators are only converted within the context of 'while' and 'for' loops:
			Chunk<K, V, Comparer> *firstEngaged = iterEngaged.next();
//JAVA TO C++ CONVERTER TODO TASK: Java iterators are only converted within the context of 'while' and 'for' loops:
			Chunk<K, V, Comparer> *firstCompacted = iterCompacted.next();

			skiplist->replace(firstEngaged->minKey,firstEngaged, firstCompacted);

			// update from infant to normal
			firstCompacted->creator = nullptr;
			Chunk::unsafe::storeFence();

			// remove all old chunks from index.
			// compacted chunks are still accessible through the first updated chunk
			while (iterEngaged != engagedChunks.end())
			{
				Chunk<K, V, Comparer> *engagedToRemove = *iterEngaged;
				skiplist->remove(engagedToRemove->minKey,engagedToRemove);
				iterEngaged++;
			}

			// for simplicity -  naive lock implementation
			// can be implemented without locks using versions on next pointer in  skiplist

			while (iterCompacted != compacted.end())
			{
				Chunk<K, V, Comparer> *compactedToAdd = *iterCompacted;

//JAVA TO C++ CONVERTER TODO TASK: Multithread locking is not converted to native C++ unless you choose one of the options on the 'Miscellaneous Options' dialog:
				synchronized(compactedToAdd)
				{
					skiplist->putIfAbsent(compactedToAdd->minKey,compactedToAdd);
					compactedToAdd->creator = nullptr;
				}
				iterCompacted++;
			}
		}

		void connectToChunkList(vector<Chunk<K, V, Comparer>*> &engaged, vector<Chunk<K, V, Comparer>*> &children)
		{

			updateLastChild(engaged,children);

			Chunk<K, V, Comparer> *firstEngaged = engaged[0];

			// replace in linked list - we now need to find previous chunk to our chunk
			// and CAS its next to point to c1, which is the same c1 for all threads who reach this point.
			// since prev might be marked (in compact itself) - we need to repeat this until successful
			while (true)
			{
				// start with first chunk (i.e., head)

				unordered_map::Entry<K, Chunk<K, V, Comparer>*> *lowerEntry = skiplist->lowerEntry(firstEngaged->minKey);

				Chunk<K, V, Comparer> *prev = lowerEntry != nullptr ? lowerEntry->getValue() : nullptr;
				Chunk<K, V, Comparer> *curr = (prev != nullptr) ? prev->next.getReference() : nullptr;

				// if didn't succeed to find preve through the skip list -- start from the head
				if (prev == nullptr || curr != firstEngaged)
				{
					prev = nullptr;
					curr = skiplist->firstEntry().getValue(); // TODO we can store&update head for a little efficiency
					// iterate until found chunk or reached end of list
					while ((curr != firstEngaged) && (curr != nullptr))
					{
						prev = curr;
						curr = curr->next.getReference();
					}
				}

				// chunk is head or not in list (someone else already updated list), so we're done with this part
				if ((curr == nullptr) || (prev == nullptr))
				{
					break;
				}

				// if prev chunk is marked - it is deleted, need to help split it and then continue
				if (prev->next.isMarked())
				{
					rebalance(prev);
					continue;
				}

				// try to CAS prev chunk's next - from chunk (that we split) into c1
				// c1 is the old chunk's replacement, and is already connected to c2
				// c2 is already connected to old chunk's next - so all we need to do is this replacement
				if ((prev->next.compareAndSet(firstEngaged, children[0], false, false)) || (!prev->next.isMarked()))
				{
					// if we're successful, or we failed but prev is not marked - so it means someone else was successful
					// then we're done with loop
					break;
				}
			}

		}

		void updateLastChild(const vector<Chunk<K, V, Comparer>>& engaged, const vector<Chunk<K, V, Comparer>>& children)
		{
			Chunk<K, V, Comparer>& lastEngaged = engaged[engaged.size() - 1];
			Chunk<K, V, Comparer>& nextToLast = lastEngaged->markAndGetNext();
			Chunk<K, V, Comparer> *lastChild = children[children.size() - 1];

			lastChild->next.compareAndSet(nullptr, nextToLast, false, false);
		}

		/**
         * publish data into thread array - use null to clear 
         */
		void publishScan(ScanData *data)
		{
			// get index of current thread
			// since thread IDs are increasing and changing, we assume threads are created one after another (sequential IDs).
			// thus, (ThreadID % MAX_THREADS) will return a unique index for each thread in range [0, MAX_THREADS)
			int idx = static_cast<int>(Thread::currentThread().getId() % MAX_THREADS);

			// publish into thread array
			scanArray[pad(idx)] = data;
		}


	public:
		virtual int debugCountKeys()
		{
			int keys = 0;
			Chunk<K, V, Comparer> *chunk = skiplist->firstEntry().getValue();

			while (chunk != nullptr)
			{
				keys += chunk->debugCountKeys();
				chunk = chunk->next.getReference();
			}
			return keys;
		}

		virtual int debugCountKeysTotal()
		{
			int keys = 0;
			Chunk<K, V, Comparer> *chunk = skiplist->firstEntry().getValue();

			while (chunk != nullptr)
			{
				keys += chunk->debugCountKeysTotal();
				chunk = chunk->next.getReference();
			}
			return keys;
		}
		virtual int debugCountDups()
		{
			int dups = 0;
			Chunk<K, V, Comparer> *chunk = skiplist->firstEntry().getValue();

			while (chunk != nullptr)
			{
				dups += chunk->debugCountDups();
				chunk = chunk->next.getReference();
			}
			return dups;
		}
		virtual void debugPrint()
		{
			Chunk<K, V, Comparer> *chunk = skiplist->firstEntry().getValue();

			while (chunk != nullptr)
			{
				wcout << L"[ ";
				chunk->debugPrint();
				wcout << L"]\t";

				chunk = chunk->next.getReference();
			}
			wcout << endl;
		}

		virtual void printDebugStats(DebugStats *ds)
		{
			wcout << L"Chunks count: " << ds->chunksCount << endl;
			wcout << endl;

			wcout << L"Sorted size: " << ds->sortedCells / ds->chunksCount << endl;
			wcout << L"Item count: " << ds->itemCount / ds->chunksCount << endl;
			wcout << L"Occupied count: " << ds->occupiedCells / ds->chunksCount << endl;
			wcout << endl;

			wcout << L"Key jumps count: " << ds->jumpKeyCount / ds->chunksCount << endl;
			wcout << L"Val jumps count: " << ds->jumpValCount / ds->chunksCount << endl;
			wcout << endl;

			wcout << L"Null items: " << ds->nulItemsCount / ds->chunksCount << endl;
			wcout << L"Removed items: " << ds->removedItems / ds->chunksCount << endl;

			wcout << L"Duplicates count: " << ds->duplicatesCount / ds->chunksCount << endl;
			wcout << endl;

		}

		virtual DebugStats *calcChunkStatistics()
		{
			Chunk<K, V, Comparer> *curr = skiplist->firstEntry().getValue();
			DebugStats *ds = new DebugStats();

			while (curr != nullptr)
			{
				curr->debugCalcCounters(ds);
				ds->chunksCount++;

				curr = curr->next.getReference();
			}

			return ds;
		}

		virtual int debugCountDuplicates()
		{
			vector<Chunk<K, V, Comparer>*> chunks = list<Chunk<K, V, Comparer>*>();
			Chunk<K, V, Comparer> *curr = skiplist->firstEntry().getValue();

			while (curr != nullptr)
			{
				//curr.debugCompacted();

				chunks.push_back(curr);
				curr = curr->next.getReference();
			}

			MultiChunkIterator<K, V> *iter = new MultiChunkIterator<K, V>(chunks);
			if (!iter->hasNext())
			{
				return -1;
			}
			iter->next();

			K prevKey = iter->getKey();
			int prevVersion = iter->getVersion();
			int duplicates = 0;
			int total = 0;
			int nullItems = 0;

			while (iter->hasNext())
			{
				total++;
				iter->next();
				K currKey = iter->getKey();
				V val = iter->getValue();

				int currVersion = iter->getVersion();

				if (currKey->equals(prevKey))
				{
					duplicates++;
				}

				if (val == nullptr)
				{
					nullItems++;
				}
				prevKey = currKey;
				prevVersion = currVersion;
				iter++;
			}



			cout << "Number of chunks: " << chunks.size() << end;
			cout << "Total number of elements: " << total << end;
			cout << "Total number of null items: " << nullItems << end;
			return duplicates;
		}


	};
    
    template<typename K, typename V, class Comparer = less<K>>
    int KiWi<K, V, Comparer>::RebalanceSize = 2;
}

#endif /* KiWi_h */
