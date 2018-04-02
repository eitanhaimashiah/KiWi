#ifndef GALOIS_WORKLIST_KIWI_CHUNK_H
#define GALOIS_WORKLIST_KIWI_CHUNK_H

#include <atomic>
#include <map>
#include <vector>
#include <iostream>
#include <cmath>
#include <limits>
#include <stdexcept>
#include "exceptionhelper.h"
#include "ThreadData.h"

// Forward class declarations
namespace kiwi { class Statistics; }
namespace kiwi { class ItemsIterator; }
namespace kiwi { class VersionsIterator; }

namespace kiwi
{

	template<typename K, typename V>
	class Chunk
	{
        using PutData = kiwi::ThreadData::PutData<K,V>;
        
    // *************	Constants			**************
	protected:
		static constexpr int NONE = 0; // constant for "no version", "no index", etc. MUST BE 0!
		static constexpr int FREEZE_VERSION = 1;
		static constexpr int CANCELED_REMOVE_NEXT = -1;

		// order_size(4) = next + version + key + data
		static constexpr int ORDER_SIZE = 4; // # of fields in each item of order array
		static constexpr int OFFSET_NEXT = 0;
		static constexpr int OFFSET_VERSION = 1; // POSITIVE means item is linked, otherwise might not be linked yet
		static constexpr int OFFSET_KEY = 2;
		static constexpr int OFFSET_DATA = 3;

		// location of the first (head) node - just a next pointer
	private:
		static constexpr int HEAD_NODE = 0;
		// index of first item in order-array, after head (not necessarily first in list!)
		static constexpr int FIRST_ITEM = 1;

	public:
		static constexpr int MAX_ITEMS = 4500;
		static constexpr bool ALLOW_DUPS = true;

    // *************	Members				**************
	//	static Unsafe *const unsafe;
	private:
		std::vector<int> const orderArray;  // array is initialized to 0, i.e., NONE - this is important!
        std::atomic<int> * const orderIndex;    // points to next free index of order array
        std::atomic<int> * const dataIndex;     // points to next free index of data array
        
        Statistics *statistics;
        int orderIndexSerial = 0;
        int dataIndexSerial = 0;
        PutData<K, V> p;
        
        std::vector<PutData<K, V>*> putArray;
        static constexpr int PAD_SIZE = 100;
        
	protected:
		std::vector<void*> const dataArray;
        int sortedCount = 0; // # of sorted items at order-array's beginning (resulting from split)
        
	public:
		K minKey; // minimal key that can be put in this chunk

		AtomicMarkableReference<Chunk<K, V>*> *next;
		AtomicReference<Rebalancer<K, V>*> *rebalancer;

		Chunk<K, V> *creator; // in split/compact process, represents parent of split (can be null!)
		AtomicReference<std::vector<Chunk<K, V>*>> *children;

		virtual ~Chunk()
		{
			delete orderIndex;
			delete dataIndex;
			delete next;
			delete rebalancer;
			delete creator;
			delete children;
			delete statistics;
		}

		int getOrderIndexSerial()
		{
			return orderIndexSerial;
		}

		int getFirstItemOrderId()
		{
			return get(HEAD_NODE,OFFSET_NEXT);
		}

		bool isFreezed()
		{
			return orderIndex->get() >= orderArray.size();
		}

		bool tryFreezeItem(int const oi)
		{
			return cas(oi,OFFSET_VERSION, NONE, FREEZE_VERSION);
		}

		virtual int copyValues(std::vector<void*> &result, int const idx, int const myVer, K const min, K const max, SortedMap<K, PutData<K, V>*> *const items) = 0;

		/// <summary>
		/// this method is used by scan operations (ONLY) to help pending put operations set a version </summary>
		/// <returns> sorted map of items matching key range of any currently-pending put operation  </returns>

		virtual SortedMap<K, PutData<K, V>*> *helpPutInScan(int myVersion, K min, K max)
		{
			SortedMap<K, PutData<K, V>*> *items = std::map<K, PutData<K, V>*>();

			// go over thread data of all threads
			for (int i = 0; i < KiWi::MAX_THREADS; ++i)
			{
				// make sure data is for a Put operatio
				PutData<K, V> *currPut = putArray[pad(i)];
				if (currPut == nullptr)
				{
					continue;
				}

				// if put operation's key is not in key range - skip it
				K currKey = readKey(currPut->orderIndex);
				if ((currKey(min) < 0) || (currKey(max) > 0))
				{
					continue;
				}

				// read the current version of the item
				int currVer = getVersion(currPut->orderIndex);

				// if empty, try to set to my version
				if (currVer == NONE)
				{
					currVer = setVersion(currPut->orderIndex, myVersion);
				}

				// if item is frozen or beyond my version - skip it
				if ((currVer == Chunk::FREEZE_VERSION) || (currVer > myVersion))
				{
					continue;
				}

				// get found item matching current key
				PutData<K, V> *item = items->get(currKey);

				// is there such an item we previously found? check if we need to replace it
				if (item != nullptr)
				{
					// get its version
					int itemVer = getVersion(item->orderIndex);

					// existing item is newer - don't replace
					if (itemVer > currVer)
					{
						continue;
					}
					// if same versions - continue checking (otherwise currVer is newer so will replace item)
					else if (itemVer == currVer)
					{
						// same chunk & version but items's index is larger - don't replace
						if (item->orderIndex > currPut->orderIndex)
						{
							continue;
						}
					}

				}

				// if we've reached here then curr is newer than item, and we replace it
				items->put(currKey, currPut);
			}

			return items;
		}


	private:
		int pad(int idx)
		{
			return (PAD_SIZE + idx*PAD_SIZE);
		}


		/// <summary>
		/// this method is used by get operations (ONLY) to help pending put operations set a version </summary>
		/// <returns> newest item matching myKey of any currently-pending put operation  </returns>
	public:
		virtual PutData<K, V> *helpPutInGet(int myVersion, K myKey)
		{
			// marks the most recent put that was found in the thread-array
			PutData<K, V> *newestPut = nullptr;
			int newestVer = Chunk::NONE;

			// go over thread data of all threads
			for (int i = 0; i < KiWi::MAX_THREADS; ++i)
			{
				// make sure data is for a Put operation
				PutData<K, V> *currPut = putArray[pad(i)];
				if (currPut == nullptr)
				{
					continue;
				}

				// if put operation's key is not same as my key - skip it
				K currKey = readKey(currPut->orderIndex);
				if (currKey(myKey) != 0)
				{
					continue;
				}

				// read the current version of the item
				int currVer = getVersion(currPut->orderIndex);

				// if empty, try to set to my version
				if (currVer == Chunk::NONE)
				{
					currVer = setVersion(currPut->orderIndex, myVersion);
				}

				// if item is frozen - skip it
				if (currVer == Chunk::FREEZE_VERSION)
				{
					continue;
				}

				// current item has newer version than newest item - replace
				if (currVer > newestVer)
				{
					newestVer = currVer;
					newestPut = currPut;
				}
				// same version for both item - check according to chunk
				else if (currVer == newestVer)
				{
						// same chunk & version but current's index is larger - it is newer
						if (currPut->orderIndex > newestPut->orderIndex)
						{
							newestPut = currPut;
						}
				}
			}

			// return item if its chunk.child1 is null, otherwise return null
			if ((newestPut == nullptr) || (isRebalanced()))
			{
				return nullptr;
			}
			else
			{
				return newestPut;
			}
		}


		/// <summary>
		/// publish data into thread array - use null to clear * </summary>
		virtual void publishPut(PutData<K, V> *data)
		{
			// get index of current thread
			// since thread IDs are increasing and changing, we assume threads are created one after another (sequential IDs).
			// thus, (ThreadID % MAX_THREADS) will return a unique index for each thread in range [0, MAX_THREADS)
			int idx = static_cast<int>(Thread::currentThread().getId() % KiWi::MAX_THREADS);

			// TODO verify the assumption about sequential IDs

			// publish into thread array
			putArray[pad(idx)] = data;
			Chunk::unsafe->storeFence();

		}

		virtual void debugCalcCounters(DebugStats *ds)
		{
			ItemsIterator *iter = itemsIterator();
			ds->sortedCells += sortedCount;
			ds->occupiedCells += orderIndex->get() / ORDER_SIZE;

			int curr = NONE;
			int prev = NONE;
			int DATA_SIZE = 1;

			int prevDataId = NONE;
			int currDataId = NONE;

			K key = nullptr;
			K prevKey = nullptr;

			while (iter->hasNext())
			{

				iter->next();
				prev = curr;
				curr = iter->current;
				prevDataId = currDataId;
				currDataId = get(curr, OFFSET_DATA);

				if (prev + ORDER_SIZE != curr)
				{
					ds->jumpKeyCount++;
				}

				if (std::abs(prevDataId) + DATA_SIZE != std::abs(currDataId))
				{
					ds->jumpValCount++;
				}

				prevKey = key;
				key = iter->getKey();

				V val = iter->getValue();
				int version = iter->getVersion();

				if (val == nullptr)
				{
					ds->nulItemsCount++;
				}

				ds->itemCount++;

				if (prevKey != nullptr)
				{
					if (key(prevKey) == 0)
					{
						ds->duplicatesCount++;
					}
					else if (val == nullptr)
					{
						ds->removedItems++;
					}
				}


				iter++;
			}
		}

		/// <summary>
		///*
		/// The class contains approximate information about chunk utilization.
		/// </summary>
	public:
		class Statistics
		{
		private:
			Chunk<K*, V*> *outerInstance;

		public:
			virtual ~Statistics()
			{
				delete dupsCount;
				delete outerInstance;
			}

			Statistics(Chunk<K, V> *outerInstance);

		private:
			AtomicInteger *dupsCount = new AtomicInteger(0);
			/// <summary>
			///*
			/// </summary>
			/// <returns> Maximum number of items the chunk can hold </returns>
		public:
			virtual int getMaxItems();

			/// <summary>
			///*
			/// </summary>
			/// <returns> Number of items inserted into the chunk </returns>
			virtual int getFilledCount();

			/// <summary>
			///*
			/// </summary>
			/// <returns> Approximate number of items chunk may contain after compaction. </returns>
			virtual int getCompactedCount();

			virtual void incrementDuplicates();

			virtual int getDuplicatesCount();
		};

		/// <summary>
		///*************	Constructors		************** </summary>
		/// <summary>
		/// Create a new chunk </summary>
		/// <param name="minKey">	minimal key to be placed in chunk, used by KiWi </param>
		/// <param name="dataItemSize">	expected avg. size (in BYTES!) of items in data-array. can be an estimate </param>
	public:
		Chunk(K minKey, int dataItemSize, Chunk<K, V> *creator) : orderArray(std::vector<int>(MAX_ITEMS * ORDER_SIZE + FIRST_ITEM)), dataArray(std::vector<void*>(MAX_ITEMS + 1)), orderIndex(new AtomicInteger(FIRST_ITEM)), dataIndex(new AtomicInteger(FIRST_ITEM)) / * index 0 in data is L"NONE" */
		{

			// allocate space for head item (only "next", starts pointing to NONE==0)
			this->orderIndexSerial = FIRST_ITEM;
			this->dataIndexSerial = FIRST_ITEM;

			// allocate space for MAX_ITEMS, and add FIRST_ITEM (size of head) for order array
			//this.orderArray = new AtomicIntegerArray(MAX_ITEMS * ORDER_SIZE + FIRST_ITEM);	// initialized to 0, i.e., NONE
			this->putArray = std::vector<PutData*>(KiWi::MAX_THREADS * (PAD_SIZE + 1));

			this->children = new AtomicReference<std::vector<Chunk<K,V>>>(nullptr);

			this->next = new AtomicMarkableReference<Chunk<K,V>>(nullptr, false);
			this->minKey = minKey;
			this->creator = creator;
			this->sortedCount = 0; // no sorted items at first
			this->rebalancer = new AtomicReference<Rebalancer<K, V>*>(nullptr); // to be updated on rebalance
			this->statistics = new Statistics(this);

			// TODO allocate space for minKey inside chunk (pointed by skiplist)?
		}

		/// <summary>
		/// static constructor - access and create a new instance of Unsafe </summary>
		private:
			class StaticConstructor
			{
			public:
				StaticConstructor();
			};

		private:
			static Chunk::StaticConstructor staticConstructor;


		/// <summary>
		///*************	Abstract Methods	************** </summary>
	public:
		virtual int allocate(K key, V data) = 0;
		virtual int allocateSerial(int key, V data) = 0;


		virtual K readKey(int orderIndex) = 0;
		virtual void *readData(int orderIndex, int dataIndex) = 0;

		/// <summary>
		/// should CLONE minKey as needed </summary>
		virtual Chunk<K, V> *newChunk(K minKey) = 0;

		/// <summary>
		///*************	Methods				************** </summary>

		virtual void finishSerialAllocation()
		{
			orderIndex->set(orderIndexSerial);
			dataIndex->set(dataIndexSerial);
		}

	public:
		class ItemsIterator
		{
		private:
			Chunk<K*, V*> *outerInstance;

		public:
			int current = 0;
			VersionsIterator *iterVersions;

			virtual ~ItemsIterator()
			{
				delete iterVersions;
				delete outerInstance;
			}

			ItemsIterator(Chunk<K, V> *outerInstance);

			bool hasNext();

			void next();

			K *getKey();

			V *getValue();

			int getVersion();

			ItemsIterator *cloneIterator();

			VersionsIterator *versionsIterator();

		public:
			class VersionsIterator
			{
			private:
				Chunk::ItemsIterator *outerInstance;

			public:
				virtual ~VersionsIterator()
				{
					delete outerInstance;
				}

				VersionsIterator(Chunk::ItemsIterator *outerInstance);

				bool justStarted = true;


				V *getValue();


				int getVersion();


				bool hasNext();


				void next();
			};

		};

	public:
		virtual ItemsIterator *itemsIterator()
		{
			return new ItemsIterator(this);
		}

		virtual ItemsIterator *itemsIterator(int oi)
		{
			ItemsIterator *iter = new ItemsIterator(this);
			iter->current = oi;

			return iter;
		}

		/// <summary>
		/// gets the data for the given item, or 'null' if it doesn't exist </summary>
		virtual V getData(int orderIndex)
		{
			// get index of data in data-array (abs- it might be negative)
			int di = get(orderIndex, OFFSET_DATA);

			// if no data for item - return null
			if (di < 0)
			{
				return nullptr;
			}
			else
			{
				return static_cast<V>(readData(orderIndex, di));
			}
		}

		virtual bool isInfant()
		{
			return creator != nullptr;
		}

		virtual bool isRebalanced()
		{
			Rebalancer<K, V> *r = getRebalancer();
			if (r == nullptr)
			{
				return false;
			}

			return r->isCompacted();

		}
		/// <summary>
		/// gets the field of specified offset for given item </summary>
	protected:
		virtual int get(int item, int offset)
		{
			return orderArray[item + offset];
		}
		/// <summary>
		/// sets the field of specified offset to 'value' for given item </summary>
		virtual void set(int item, int offset, int value)
		{
			orderArray[item + offset] = value;
		}

		/// <summary>
		/// performs CAS from 'expected' to 'value' for field at specified offset of given item </summary>
	private:
		bool cas(int item, int offset, int expected, int value)
		{
			return unsafe->compareAndSwapInt(orderArray, Unsafe::ARRAY_INT_BASE_OFFSET + (item + offset) * Unsafe::ARRAY_INT_INDEX_SCALE, expected, value);
		}

		/// <summary>
		/// binary search for largest-entry smaller than 'key' in sorted part of order-array. </summary>
		/// <returns> the index of the entry from which to start a linear search -
		/// if key is found, its previous entry is returned!  </returns>
		int binaryFind(K key)
		{
			// if there are no sorted keys,or the first item is already larger than key -
			// return the head node for a regular linear search
			if ((sortedCount == 0) || (readKey(FIRST_ITEM)(key) >= 0))
			{
				return HEAD_NODE;
			}

			// TODO check last key to avoid binary search?

			int start = 0;
			int end = sortedCount;

			while (end - start > 1)
			{
				int curr = start + (end - start) / 2;

				if (readKey(curr * ORDER_SIZE + FIRST_ITEM)(key) >= 0)
				{
					end = curr;
				}
				else
				{
					start = curr;
				}
			}

			return start * ORDER_SIZE + FIRST_ITEM;
		}

		/// <summary>
		///*
		/// Engage the chunk to a rebalancer r.
		/// </summary>
		/// <param name="r"> -- a rebalancer to engage with </param>
		/// <returns> rebalancer engaged with the chunk </returns>
	public:
		virtual Rebalancer *engage(Rebalancer *r)
		{
			rebalancer->compareAndSet(nullptr,r);
			return rebalancer->get();
		}

		/// <summary>
		///*
		/// Checks whether the chunk is engaged with a given rebalancer. </summary>
		/// <param name="r"> -- a rebalancer object. If r is null, verifies that the chunk is not engaged to any rebalancer </param>
		/// <returns> true if the chunk is engaged with r, false otherwise </returns>
		virtual bool isEngaged(Rebalancer *r)
		{
			return rebalancer->get() == r;
		}

		/// <summary>
		///*
		/// Fetch a rebalancer engaged with the chunk. </summary>
		/// <returns> rebalancer object or null if not engaged. </returns>
		virtual Rebalancer *getRebalancer()
		{
			return rebalancer->get();
		}

		/// <summary>
		///*
		/// </summary>
		/// <returns> statistics object containing approximate utilization information. </returns>
		virtual Statistics *getStatistics()
		{
			return statistics;
		}

		/// <summary>
		/// marks this chunk's next pointer so this chunk is marked as deleted </summary>
		/// <returns> the next chunk pointed to once marked (will not change)  </returns>
		virtual Chunk<K, V> *markAndGetNext()
		{
			// new chunks are ready, we mark frozen chunk's next pointer so it won't change
			// since next pointer can be changed by other split operations we need to do this in a loop - until we succeed
			while (true)
			{
				// if chunk is marked - that is ok and its next pointer will not be changed anymore
				// return whatever chunk is set as next
				if (next->isMarked())
				{
					return next->getReference();
				}
				// otherwise try to mark it
				else
				{
					// read chunk's current next
					Chunk<K, V> *savedNext = next->getReference();

					// try to mark next while keeping the same next chunk - using CAS
					// if we succeeded then the next pointer we remembered is set and will not change - return it
					if (next->compareAndSet(savedNext, savedNext, false, true))
					{
						return savedNext;
					}
				}
			}
		}

		/// <summary>
		/// freezes chunk so no more changes can be done in it. also marks pending items as frozen </summary>
		/// <returns> number of items in this chunk  </returns>
		virtual void freeze()
		{
			int numItems = 0;

			// prevent new puts to the chunk
			orderIndex->addAndGet(orderArray.size());

			// go over thread data of all threads
			for (int i = 0; i < KiWi::MAX_THREADS; ++i)
			{
				// make sure data is for a Put operatio
				PutData<K, V> *currPut = putArray[pad(i)];
				if (currPut == nullptr)
				{
					continue;
				}
	/*
				if(currPut.chunk != this)
				{
					continue;
				}
	*/
				int idx = currPut->orderIndex;
				int version = getVersion(idx);

				// if item is frozen, ignore it - so only handle non-frozen items
				if (version != FREEZE_VERSION)
				{
					// if item has no version, try to freeze it
					if (version == NONE)
					{
						// set version to FREEZE so put op knows to restart
						// if succeded - item will not be in this chunk, we can continue to next item
						if ((cas(idx, OFFSET_VERSION, NONE, FREEZE_VERSION)) || (getVersion(idx) == FREEZE_VERSION))
						{
							continue;
						}
					}

					// if we reached here then item has a version - we need to help by adding item to chunk's list
					// we need to help the pending put operation add itself to the list before proceeding
					// to make sure a frozen chunk is actually frozen - all items are fully added
					addToList(idx, readKey(idx));

				}
			}

		}

		/// <summary>
		/// finds and returns the index of the first item that is equal or larger-than the given min key
		/// with max version that is equal or less-than given version.
		/// returns NONE if no such key exists 
		/// </summary>
		virtual int findFirst(K minKey, int version)
		{
			// binary search sorted part of order-array to quickly find node to start search at
			// it finds previous-to-key so start with its next
			int curr = get(binaryFind(minKey), OFFSET_NEXT);

			// iterate until end of list (or key is found)
			while (curr != NONE)
			{
				K key = readKey(curr);

				// if item's key is larger or equal than min - we've found a matching key
				if (key(minKey) >= 0)
				{
					// check for valid version
					if (getVersion(curr) <= version)
					{
						return curr;
					}
				}

				curr = get(curr, OFFSET_NEXT);
			}

			return NONE;
		}

		/// <summary>
		/// returns the index of the first item in this chunk with a version <= version </summary>
		virtual int getFirst(int version)
		{
			int curr = get(HEAD_NODE, OFFSET_NEXT);

			// iterate over all items
			while (curr != NONE)
			{
				// if current item is of matching version - return it
				if (getVersion(curr) <= version)
				{
					return curr;
				}

				// proceed to next item
				curr = get(curr, OFFSET_NEXT);
			}
			return NONE;
		}

		int findNext(int curr, int version, K key)
		{
			curr = get(curr, OFFSET_NEXT);

			while (curr != NONE)
			{
				K currKey = readKey(curr);

				// if in a valid version, and a different key - found next item
				if ((currKey(key) != 0) && (getVersion(curr) <= version))
				{
					return curr;
				}

				// otherwise proceed to next
				curr = get(curr, OFFSET_NEXT);
			}

			return NONE;
		}

		/// <summary>
		/// finds and returns the value for the given key, or 'null' if no such key exists </summary>
		virtual V find(K key, PutData<K, V> *item)
		{
			// binary search sorted part of order-array to quickly find node to start search at
			// it finds previous-to-key so start with its next
			int curr = get(binaryFind(key), OFFSET_NEXT);

			// iterate until end of list (or key is found)
			while (curr != NONE)
			{
				// compare current item's key to searched key
				int cmp = readKey(curr)(key);

				// if item's key is larger - we've exceeded our key
				// it's not in chunk - no need to search further
				if (cmp > 0)
				{
					return nullptr;
				}
				// if keys are equal - we've found the item
				else if (cmp == 0)
				{
					return chooseNewer(curr, item);
				}
				// otherwise- proceed to next item
				else
				{
					curr = get(curr, OFFSET_NEXT);
				}
			}

			return nullptr;
		}

	private:
		V chooseNewer(int item, PutData<K, V> *pd)
		{
			// if pd is empty or in different chunk, then item is definitely newer
			// it's true since put() publishes after finding a chunk, and get() finds chunk only after reading thread-array
			// so get() definitely sees the same chunks put() sees, or NEWER chunks
			if ((pd == nullptr))
			{
					//|| (pd.chunk != this)
				return getData(item);
			}

			// if same chunk then regular comparison (version, then orderIndex)
			int itemVer = getVersion(item);
			int dataVer = getVersion(pd->orderIndex);

			if (itemVer > dataVer)
			{
				return getData(item);
			}
			else if (dataVer > itemVer)
			{
				return getData(pd->orderIndex);
			}
			else
			{
				// same version - return latest item by order in order-array
				return getData(std::max(item, pd->orderIndex));
			}
		}

		/// <summary>
		/// add the given item (allocated in this chunk) to the chunk's linked list </summary>
		/// <param name="orderIndex"> index of item in order-array </param>
		/// <param name="key"> given for convenience  </param>
	public:
		void addToList(int const orderIndex, K key)
		{
			int prev, curr;
			int ancor = -1;

			// retry adding to list until successful
			// no items are removed from list - so we don't need to restart on failures
			// so if we CAS some node's next and fail, we can continue from it
			// --retry so long as version is negative (which means item isn't in linked-list)
			while (get(orderIndex, OFFSET_VERSION) < 0)
			{
				// remember next pointer in entry we're trying to add
				int savedNext = get(orderIndex, OFFSET_NEXT);

				 // start iterating from quickly-found node (by binary search) in sorted part of order-array
				if (ancor == -1)
				{
					ancor = binaryFind(key);
				}
				curr = ancor;

				int cmp = -1;

				// iterate items until key's position is found
				while (true)
				{
					prev = curr;
					curr = get(prev, OFFSET_NEXT); // index of next item in list

					// if no item, done searching - add to end of list
					if (curr == NONE)
					{
						break;
					}

					// if found item we're trying to insert - already inserted by someone else, so we're done
					if (curr == orderIndex)
					{
						return;
					}
						//TODO also update version to positive?

					// compare current item's key to ours
					cmp = readKey(curr)(key);

					// if current item's key is larger, done searching - add between prev and curr
					if (cmp > 0)
					{
						break;
					}

					// if same key - check according to version and location in array
					if (cmp == 0)
					{
						// if duplicate values aren't allowed - do not add value
						if (!ALLOW_DUPS)
						{
							return;
						}

						int verMine = getVersion(orderIndex);
						int verNext = getVersion(curr);

						// if current item's version is smaller, done searching - larger versions are first in list
						if (verNext < verMine)
						{
							break;
						}

						// same versions but i'm later in chunk's array - i'm first in list
						if (verNext == verMine)
						{
							int newDataIdx = get(orderIndex, OFFSET_DATA);
							int oldDataIdx = get(curr, OFFSET_DATA);

							while ((std::abs(newDataIdx) > std::abs(oldDataIdx)) && !cas(curr,OFFSET_DATA,oldDataIdx, newDataIdx))
							{
								oldDataIdx = get(curr,OFFSET_DATA);
							}

							return;
						}
					}
				}

				if (savedNext == CANCELED_REMOVE_NEXT)
				{
					return;
				}
				if (cmp != 0 && savedNext == NONE && get(orderIndex, OFFSET_DATA) < 0)
				{
					if (cas(orderIndex,OFFSET_NEXT,savedNext, CANCELED_REMOVE_NEXT))
					{
						return;
					}
					else
					{
						continue;
					}
				}

				// try to CAS update my next to current item ("curr" variable)
				// using CAS from saved next since someone else might help us
				// and we need to avoid race conditions with other put ops and helpers
				if (cas(orderIndex, OFFSET_NEXT, savedNext, curr))
				{
					// try to CAS curr's next to point from "next" to me
					// if successful - we're done, exit loop. Otherwise retry (return to "while true" loop)
					if (cas(prev, OFFSET_NEXT, curr, orderIndex))
					{
						// if some CAS failed we restart, if both successful - we're done
						// update version to positive (getVersion() always returns positive number) to mark item is linked
						set(orderIndex, OFFSET_VERSION, getVersion(orderIndex));

						// if adding version for existing key -- update duplicates statistics
						if (cmp == 0)
						{
							statistics->incrementDuplicates();
						}

						break;
					}
				}
			}
		}

	private:
		bool casData(int curr, void *currData, void *data)
		{

			return unsafe->compareAndSwapObject(dataArray, Unsafe::ARRAY_OBJECT_BASE_OFFSET + curr * Unsafe::ARRAY_OBJECT_INDEX_SCALE, currData, data);

		}


		/// <summary>
		///*
		/// Appends a new item to the end of items array. The function assumes that the array is sorted in ascending order by (Key, -Version)
		/// The method is not thread safe!!!  Should be called for  chunks accessible by single thread only.
		/// </summary>
		/// <param name="key"> the key of the new item </param>
		/// <param name="value"> the value of the new item </param>
		/// <param name="version"> the version of the new item </param>
	public:
		void appendItem(int key, V value, int version)
		{
			int oiDest = allocateSerial(key,value);

			// update to item's version (since allocation gives NONE version)
			// version is positive so item is marked as linked
			set(oiDest, Chunk::OFFSET_VERSION, version);

			// update binary searches range
			sortedCount++;

			// handle adding of first item to empty chunk
			int prev = oiDest - ORDER_SIZE;
			if (prev < 0)
			{
				set(HEAD_NODE,OFFSET_NEXT,oiDest);
				return;
			}

			// updated dest chunk's linked list with new item
			set(prev, OFFSET_NEXT, oiDest);

		}

		virtual int getNumOfItems()
		{
			return orderIndex->get() / ORDER_SIZE;
		}

		int getNumOfItemsSerial()
		{
			return orderIndexSerial / ORDER_SIZE;
		}

		/// <summary>
		///*
		/// Copies items from srcChunk performing compaction on the fly. </summary>
		/// <param name="srcChunk"> -- chunk to copy from </param>
		/// <param name="oi"> -- start position for copying </param>
		/// <param name="maxCapacity"> -- max number of items "this" chunk can contain after copy </param>
		/// <returns> order index of next to the last copied item, NONE if all items were copied </returns>
		int copyPart(Chunk<K, V> *srcChunk, int oi, int maxCapacity, ScanIndex<K> *scanIndex)
		{


			int maxIdx = maxCapacity*ORDER_SIZE + 1;

			if (orderIndexSerial >= maxIdx)
			{
				return oi;
			}

			assert(oi < orderArray.size() - ORDER_SIZE);

			if (orderIndexSerial != FIRST_ITEM)
			{
				set(orderIndexSerial - ORDER_SIZE, OFFSET_NEXT, orderIndexSerial);
			}
			else
			{
				set(HEAD_NODE,OFFSET_NEXT,FIRST_ITEM);
			}

			int sortedSize = srcChunk->sortedCount*ORDER_SIZE + 1;
			int orderStart = oi;
			int orderEnd = orderStart - 1;

			int currKey = NONE;
			int prevKey = std::numeric_limits<int>::min();

			int currDataId = NONE;
			int prevDataId = NONE;

			int currVersion = NONE;

			int oiPrev = NONE;

			bool isFirst = true;

			while (true)
			{
				currKey = srcChunk->get(oi,OFFSET_KEY);
				currDataId = srcChunk->get(oi, OFFSET_DATA);

				int itemsToCopy = orderEnd - orderStart + 1;

				if ((currDataId > 0) && (isFirst || ((oiPrev < sortedSize) && (prevKey != currKey) && (oiPrev + ORDER_SIZE == oi) && (orderIndexSerial + itemsToCopy*ORDER_SIZE <= maxIdx) && (prevDataId + 1 == currDataId))))
				{
					orderEnd++;
					isFirst = false;

					prevKey = currKey;
					oiPrev = oi;
					oi = srcChunk->get(oi, OFFSET_NEXT);
					prevDataId = currDataId;

					if (oi != NONE)
					{
						continue;
					}

				}

				// copy continuous interval by arrayCopy
				itemsToCopy = orderEnd - orderStart + 1;
				//System.arraycopy(srcChunk.orderArray, orderStart, orderArray, orderIndexSerial, itemsToCopy*ORDER_SIZE );
				if (itemsToCopy > 0)
				{
					for (int i = 0; i < itemsToCopy; ++i)
					{
						int offset = i * ORDER_SIZE;
						int oIdx = orderIndexSerial;

						// next should point to the next item
						orderArray[oIdx + offset + OFFSET_NEXT] = oIdx + offset + ORDER_SIZE;
						orderArray[oIdx + offset + OFFSET_VERSION] = std::abs(srcChunk->orderArray[orderStart + offset + OFFSET_VERSION]);
						orderArray[oIdx + offset + OFFSET_DATA] = dataIndexSerial + i;
						orderArray[oIdx + offset + OFFSET_KEY] = srcChunk->orderArray[orderStart + offset + OFFSET_KEY];
					}

					orderIndexSerial = orderIndexSerial + itemsToCopy * ORDER_SIZE;

					int dataIdx = srcChunk->get(orderStart, OFFSET_DATA);

					if (itemsToCopy == 1)
					{
						dataArray[dataIndexSerial] = srcChunk->dataArray[dataIdx];
					}
					else
					{

						System::arraycopy(srcChunk->dataArray, dataIdx, dataArray, dataIndexSerial, itemsToCopy);
					}

					dataIndexSerial = dataIndexSerial + itemsToCopy;
				}

				scanIndex->reset(currKey);
				//first item already copied or null
				scanIndex->savedVersion(NONE);

				currVersion = srcChunk->getVersion(oi);

				int removedVersion = NONE;

				// the case when we start from deleted item
				if (prevKey != currKey && currDataId < 0)
				{
					// remove the item if it doesn't have versions to keep
					removedVersion = currVersion;

					// move to next item
					oiPrev = oi;
					prevDataId = currDataId;

					oi = srcChunk->get(oi, OFFSET_NEXT);
					currDataId = srcChunk->get(oi,OFFSET_DATA);

					currVersion = srcChunk->getVersion(oi);
					prevKey = currKey;
					currKey = srcChunk->get(oi, OFFSET_KEY);
				}

				// copy versions of currKey if required by scanIndex, or skip to next key
				while (oi != NONE && prevKey == currKey)
				{
						if (scanIndex->shouldKeep(currVersion))
						{
							if (currDataId < 0)
							{
								removedVersion = currVersion;
								scanIndex->savedVersion(currVersion);
							}
							else if (currVersion != removedVersion)
							{
								if (removedVersion != NONE)
								{
									appendItem(currKey, nullptr, removedVersion);
									set(orderIndexSerial - ORDER_SIZE, OFFSET_NEXT, orderIndexSerial);
									scanIndex->savedVersion(removedVersion);
									removedVersion = NONE;
								}


								appendItem(currKey, static_cast<V>(srcChunk->dataArray[currDataId]), currVersion);
								set(orderIndexSerial - ORDER_SIZE, OFFSET_NEXT, orderIndexSerial);
								scanIndex->savedVersion(currVersion);
							}
						}

						oiPrev = oi;
						prevDataId = currDataId;

						oi = srcChunk->get(oi, OFFSET_NEXT);
						currDataId = srcChunk->get(oi,OFFSET_DATA);

						currVersion = srcChunk->getVersion(oi);
						prevKey = currKey;
						currKey = srcChunk->get(oi, OFFSET_KEY);
				}

				if (oi == NONE || orderIndexSerial > maxIdx)
				{
					break;
				}


				orderStart = oi;
				orderEnd = orderStart - 1;
				isFirst = true;
			}


			int setIdx = orderIndexSerial > FIRST_ITEM ? orderIndexSerial - ORDER_SIZE : HEAD_NODE;
			set(setIdx,OFFSET_NEXT, NONE);

			orderIndex->set(orderIndexSerial);
			dataIndex->set(dataIndexSerial);
			sortedCount = orderIndexSerial / ORDER_SIZE;

			return oi;
		}

		/// <summary>
		/// base allocate method for use in allocation by implementing classes </summary>
		/// <returns> index of allocated order-array item (can be used to get data-array index)  </returns>
	protected:
		int baseAllocate(int dataSize)
		{
			// increment order array to get new index in it
			int oi = orderIndex->getAndAdd(ORDER_SIZE);
			if (oi + ORDER_SIZE > orderArray.size())
			{
				return -1;
			}

			// increment data array to get new index in it
			int di = dataIndex->getAndIncrement();
			if (di >= dataArray.size())
			{
				return -1;
			}

			// if there's data - allocate room for it
			// otherwise DATA field of order-item is left as NONE
			// write base item data location (offset of data-array) to order-array
			// since NONE==0, item's version and next are already set to NONE
			di = dataSize > 0? di : -di;

			set(oi, OFFSET_DATA, di);

			// return index of allocated order-array item
			return oi;
		}

		int baseAllocateSerial(int dataSize)
		{

			int oi = orderIndexSerial;
			orderIndexSerial += ORDER_SIZE;

			if (oi + ORDER_SIZE > orderArray.size())
			{
				return -1;
			}

			// if there's data - allocate room for it
			// otherwise DATA field of order-item is left as NONE
			// increment data array to get new index in it
			int di = dataIndexSerial;
			if (di >= dataArray.size())
			{
				return -1;
			}

			di = dataSize > 0 ? di : -di;

			dataIndexSerial++;

			// write base item data location (offset of data-array) to order-array
			// since NONE==0, item's version and next are already set to NONE
			set(oi, OFFSET_DATA, di);

			// return index of allocated order-array item
			return oi;

		}

		/// <summary>
		/// gets the current version of the given order-item </summary>
	public:
		virtual int getVersion(int orderIndex)
		{
			return std::abs(get(orderIndex, OFFSET_VERSION));
		}
		/// <summary>
		/// tries to set (CAS) the version of order-item to specified version </summary>
		/// <returns> whatever version is successfuly set (by this thread or another)	  </returns>
		virtual int setVersion(int orderIndex, int version)
		{
			// try to CAS version from NO_VERSION to desired version
			if (cas(orderIndex, OFFSET_VERSION, NONE, -version))
			{
				return version;
			}
			// if failed (someone else's CAS succeeded) - read version again and return it
			else
			{
				return getVersion(orderIndex);
			}
		}

		virtual int debugCountKeysTotal()
		{
			int curr = get(HEAD_NODE, OFFSET_NEXT);
			//int prev = curr;
			int keys = 0;

			if (curr != NONE)
			{
				keys = 1;
				curr = get(curr,OFFSET_NEXT);

				while (curr != NONE)
				{
					//if (readKey(curr).compareTo(readKey(prev)) != 0)
					++keys;
					//prev = curr;
					curr = get(curr,OFFSET_NEXT);
				}
			}
			return keys;
		}

		virtual int debugCountKeys()
		{
			int curr = get(HEAD_NODE, OFFSET_NEXT);
			int prev = curr;
			int keys = 0;

			if (curr != NONE)
			{
				keys = 1;
				curr = get(curr,OFFSET_NEXT);

				while (curr != NONE)
				{
					if (readKey(curr)(readKey(prev)) != 0)
					{
						++keys;
					}
					prev = curr;
					curr = get(curr,OFFSET_NEXT);
				}
			}
			return keys;
		}
		virtual int debugCountDups()
		{
			int curr = get(HEAD_NODE, OFFSET_NEXT);
			int prev = curr;
			int dups = 0;

			if (curr != NONE)
			{
				curr = get(curr,OFFSET_NEXT);

				while (curr != NONE)
				{
					if (readKey(curr)(readKey(prev)) == 0)
					{
						++dups;
					}
					prev = curr;
					curr = get(curr,OFFSET_NEXT);
				}
			}
			return dups;
		}

		virtual void debugCompacted()
		{
			int prevKey = get(FIRST_ITEM,OFFSET_KEY);
			int prevVersion = get(FIRST_ITEM, OFFSET_VERSION);
			int prevNext = get(FIRST_ITEM, OFFSET_NEXT);

			if (get(HEAD_NODE,OFFSET_NEXT) != FIRST_ITEM)
			{
				throw IllegalStateException();
			}

			for (int oi = FIRST_ITEM + ORDER_SIZE; oi < orderArray.size() - ORDER_SIZE; oi += ORDER_SIZE)
			{
				int key = get(oi,OFFSET_KEY);
				int version = get(oi, OFFSET_VERSION);

				if (prevKey >= key)
				{
					throw IllegalStateException();
				}

				if (prevNext != oi)
				{
					throw IllegalStateException();
				}
				prevKey = key;
				prevVersion = version;
				prevNext = get(oi, OFFSET_NEXT);

				if (prevNext == NONE)
				{
					break;
				}
			}
		}

		virtual void debugPrint()
		{
			std::wcout << this->minKey << L" :: ";
			int curr = get(HEAD_NODE, OFFSET_NEXT);

			while (curr != NONE)
			{
				std::wcout << L"(" << readKey(curr) << L"," << getData(curr) << L"," << curr << L") ";
				curr = get(curr, OFFSET_NEXT);
			}
		}
	};

}

#endif
