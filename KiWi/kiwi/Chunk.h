#ifndef Chunk_h
#define Chunk_h

#include <atomic>
#include <memory>
#include <map>
#include <vector>
#include <iostream>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <thread>

#include "PutData.h"
#include "DebugStats.h"
#include "ScanIndex.h"

#include "exceptionhelper.h"
#include "MarkableReference.h"
//#include "../util/MarkableReference.h" // TODO: Check in Galios how they include such files

// Class forward declarations
namespace kiwi { template<typename K, typename V, class Comparer> class Rebalancer; }
namespace kiwi { template<typename K, typename V, class Comparer> class MultiChunkIterator; }

using namespace std;

namespace kiwi
{
	template<typename K, typename V, class Comparer = less<K>>
	class Chunk
	{
        friend class  MultiChunkIterator<K, V, Comparer>; // Friend class forward declaration
        
    /***************	Nested Classes  	***************/
    public:
        /**
         * The class contains approximate information about chunk utilization.
         */
        class Statistics
        {
        private:
            Chunk<K, V, Comparer>& chunk;
            atomic<int> dupsCount;
        public:
            
            Statistics(Chunk<K, V, Comparer>& chunk): chunk(chunk), dupsCount(0) {}
            
            /**
             * @return Maximum number of items the chunk can hold
             */
            virtual int getMaxItems()
            {
                return Chunk::MAX_ITEMS;
            }
            
            /**
             * @return Number of items inserted into the chunk
             */
            virtual int getFilledCount()
            {
                return chunk.orderIndex.load() / Chunk::ORDER_SIZE;
            }
            
            /**
             * @return Approximate number of items chunk may contain after compaction.
             */
            virtual int getCompactedCount()
            {
                return getFilledCount() - getDuplicatesCount();
            }
            
            virtual void incrementDuplicates()
            {
                dupsCount.fetch_add(1);
            }
            
            virtual int getDuplicatesCount()
            {
                return dupsCount.load();
            }
            
        };
        
        class ItemsIterator
        {
        public:
            class VersionsIterator
            {
            private:
                Chunk<K, V, Comparer>& chunk;
                Chunk::ItemsIterator& items;
                bool justStarted;
                
            public:
                VersionsIterator(Chunk::ItemsIterator& items) : items(items), justStarted(true), chunk(items.chunk) {}
                
                V getValue()
                {
                    return chunk.getData(items.current);
                }
                
                int getVersion()
                {
                    return chunk.getVersion(items.current);
                }
                
                bool hasNext()
                {
                    if (justStarted) return true;
                    int next = chunk.get(items.current, OFFSET_NEXT);
                    
                    if (next == Chunk::NONE) return false;
                    return chunk.compare(chunk.readKey(items.current), chunk.readKey(next)) == 0;
                }
                
                void next()
                {
                    if (justStarted)
                    {
                        justStarted = false;
                        return;
                    }
                    else
                    {
                        items.current = chunk.get(items.current, OFFSET_NEXT);
                    }
                }
                
            };
            
        public:
            Chunk<K, V, Comparer>& chunk;
            int current = 0;
            VersionsIterator iterVersions;
            
            ItemsIterator(Chunk<K, V, Comparer>& chunk) : chunk(chunk), current(HEAD_NODE), iterVersions(*this) {}
            
            ItemsIterator(Chunk<K, V, Comparer>& chunk, int oi) : chunk(chunk), current(oi), iterVersions(*this) {}
            
            ItemsIterator(const ItemsIterator& other) : chunk(other.chunk), current(other.current), iterVersions(*this) {}
            
            bool hasNext()
            {
                return chunk.get(current, OFFSET_NEXT) != Chunk::NONE;
            }
            
            void next()
            {
                current = chunk.get(current, OFFSET_NEXT);
                iterVersions.justStarted = true;
            }
            
            K getKey()
            {
                return chunk.readKey(current);
            }
            
            V getValue()
            {
                return chunk.getData(current);
            }
            
            int getVersion()
            {
                return chunk.getVersion(current);
            }
            
            ItemsIterator cloneIterator()
            {
                ItemsIterator newIterator (chunk);
                newIterator.current = this->current;
                return newIterator;
            }
            
            VersionsIterator& versionsIterator()
            {
                return iterVersions;
            }
        };
        
    /***************	Constants			***************/
	public:
		static constexpr int NONE = 0; // constant for "no version", "no index", etc. MUST BE 0!
		static constexpr int FREEZE_VERSION = 1;
		static constexpr int CANCELED_REMOVE_NEXT = -1;

        // TODO: Replace with appropriate struct
		// order_size(4) = next + version + key + data
		static constexpr int ORDER_SIZE = 4; // # of fields in each item of order array
		static constexpr int OFFSET_NEXT = 0;
		static constexpr int OFFSET_VERSION = 1; // POSITIVE means item is linked, otherwise might not be linked yet
		static constexpr int OFFSET_KEY = 2;
		static constexpr int OFFSET_DATA = 3;
        
        static constexpr int MAX_ITEMS = 4500;
        static constexpr bool ALLOW_DUPS = true;
        
	private:
        // location of the first (head) node - just a next pointer
        static constexpr int HEAD_NODE = 0;
        
		// index of first item in order-array, after head (not necessarily first in list!)
		static constexpr int FIRST_ITEM = 1;
        
        static constexpr int PAD_SIZE = 100;
        
        // TODO: Taken from the KiWi class, think of better idea...
        static constexpr int MAX_THREADS = 32;

    /***************	Members				***************/
	//	static Unsafe *const unsafe;
	private:
        // TODO: Understand what's the best way to treat orderArray in the sense of atomic<> etc.
        // Moreover,the use of vector<> should be considerd again.
		vector<int> orderArray;    // array is initialized to 0, i.e., NONE - this is important!
        atomic<int> orderIndex;    // points to next free index of order array
        atomic<int> dataIndex;     // points to next free index of data array
        
        Statistics statistics;
        int orderIndexSerial = 0;
        int dataIndexSerial = 0;
        
        vector<shared_ptr<PutData>> putArray; // TODO: Be sure that vector/shared_ptr is right here
        
        Comparer compare;
        
	protected:
		vector<void*> const dataArray;
        
	public:
        int sortedCount; // # of sorted items at order-array's beginning (resulting from split)
        
		K minKey; // minimal key that can be put in this chunk
        atomic<MarkableReference<Chunk<K, V, Comparer>>> next;     // TODO: Maybe I should replace it with std::pair
		atomic<shared_ptr<Rebalancer<K, V, Comparer>>> rebalancer; // TODO: shared_ptr has a solution from atomic, use it!

		shared_ptr<Chunk<K, V, Comparer>> creator; // in split/compact process, represents parent of split (can be null!)
        
         // TODO: Be sure that unique_ptr is right here.
         // In addition, probably atomic on vector is not a good practice.
		atomic<unique_ptr<vector<Chunk<K, V, Comparer>>>> children;

    /***************	Constructors		***************/
	public:
        /**
         * Creates a new chunk
         * @param minKey	minimal key to be placed in chunk, used by KiWi
         * @param dataItemSize	expected avg. size (in BYTES!) of items in data-array. can be an estimate
         */
		Chunk(K minKey, int dataItemSize, shared_ptr<Chunk<K, V, Comparer>> creator) :
        orderArray(MAX_ITEMS * ORDER_SIZE + FIRST_ITEM), // allocate space for MAX_ITEMS, and add FIRST_ITEM (size of head) for order array
        dataArray(MAX_ITEMS + 1),
        orderIndex(FIRST_ITEM),
        dataIndex(FIRST_ITEM),
        orderIndexSerial(FIRST_ITEM),                    // allocate space for head item (only "next", starts pointing to NONE==0)
        dataIndexSerial(FIRST_ITEM),
        children(nullptr),
        next(nullptr, false),
        minKey(minKey),
        creator(creator),
        sortedCount(0),         // no sorted items at first
        rebalancer(nullptr),    // to be updated on rebalance
        statistics(*this)
		{
			// TODO: allocate space for minKey inside chunk (pointed by skiplist)?
		}
        
        virtual ~Chunk() {}
        
    /***************	Abstract Methods	***************/
    public:
        virtual int allocate(K key, V data) = 0;
        virtual int allocateSerial(int key, V data) = 0;
        
        
        virtual K readKey(int orderIndex) = 0;
        virtual void *readData(int orderIndex, int dataIndex) = 0;
        
        /** should CLONE minKey as needed */
        virtual Chunk<K, V, Comparer> *newChunk(K minKey) = 0;
        
    /***************	Methods				***************/
    public:
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
            return orderIndex.load() >= orderArray.size();
        }
        
        bool tryFreezeItem(int const oi)
        {
            return cas(oi,OFFSET_VERSION, NONE, FREEZE_VERSION);
        }
        
        // TODO: Check if K has compare() function
        virtual int copyValues(vector<void*>& result, int const idx, int const myVer, const K min, const K max, map<K, PutData> const& items) = 0;
        
        /**
         * This method is used by scan operations (ONLY) to help pending put operations set a version
         * @return sorted map of items matching key range of any currently-pending put operation
         */
        virtual map<K, unique_ptr<PutData>> helpPutInScan(int myVersion, K min, K max)
        {
            map<K, unique_ptr<PutData>> items;
            
            // go over thread data of all threads
            for (int i = 0; i < MAX_THREADS; ++i)
            {
                // make sure data is for a Put operation
                shared_ptr<PutData> currPut = putArray[pad(i)];
                if (currPut == nullptr)
                {
                    continue;
                }
                
                // if put operation's key is not in key range - skip it
                K currKey = readKey(currPut->orderIndex);
				if ((compare(currKey, min) < 0) || (compare(currKey, max) > 0))
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
                auto it = items.find(currKey);
                
                // is there such an item we previously found? check if we need to replace it
                if (it != items.end())
                {
                    unique_ptr<PutData> item = it->second;
                    
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
                items[currKey] = currPut;
            }
            
            return items; // TODO: Verify it's the proper way to do that
        }
        
        
    private:
        
        // TODO: Check this function in the context of orderArray
        int pad(int idx)
        {
            return (PAD_SIZE + idx*PAD_SIZE);
        }
        
    public:
        /**
         * This method is used by get operations (ONLY) to help pending put operations set a version
         * @return newest item matching myKey of any currently-pending put operation
         */
        virtual shared_ptr<PutData> helpPutInGet(int myVersion, K myKey)
        {
            // marks the most recent put that was found in the thread-array
            shared_ptr<PutData> newestPut = nullptr;
            int newestVer = NONE;
            
            // go over thread data of all threads
            for (int i = 0; i < MAX_THREADS; ++i)
            {
                // make sure data is for a Put operation
                shared_ptr<PutData> currPut = putArray[pad(i)];
                if (currPut == nullptr)
                {
                    continue;
                }
                
                // if put operation's key is not same as my key - skip it
                K currKey = readKey(currPut->orderIndex);
				if (compare(currKey, min) != 0)
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
                
                // if item is frozen - skip it
                if (currVer == FREEZE_VERSION)
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
        
        
        /** Publishes data into thread array - use null to clear **/
        virtual void publishPut(const shared_ptr<PutData>& data)
        {
            // TODO: I should create a thread map in a sperate class mapping thread::id to numbers
            // https://stackoverflow.com/questions/15708983/how-can-you-get-the-linux-thread-id-of-a-stdthread
            // https://stackoverflow.com/questions/7432100/how-to-get-integer-thread-id-in-c11
            
            // get index of current thread
            // since thread IDs are increasing and changing, we assume threads are created one after another (sequential IDs).
            // thus, (ThreadID % MAX_THREADS) will return a unique index for each thread in range [0, MAX_THREADS)
            int idx = 1; // static_cast<int>(this_thread::get_id()) % MAX_THREADS;
            
            // publish into thread array
            putArray[pad(idx)] = data;
            atomic_thread_fence(memory_order_release); // TODO: verify it indeed equals to Chunk.unsafe.storeFence();
        }
        
        virtual void debugCalcCounters(DebugStats& ds)
        {
            ItemsIterator iter = itemsIterator();
            ds.sortedCells += sortedCount;
            ds.occupiedCells += orderIndex.load() / ORDER_SIZE; // TODO: Check this line after orderArray task
            
            int curr = NONE;
            int prev = NONE;
            int DATA_SIZE = 1;
            
            int prevDataId = NONE;
            int currDataId = NONE;
            
            K key = nullptr;
            K prevKey = nullptr;
            
            while (iter.hasNext())
            {
                iter.next();
                prev = curr;
                curr = iter.current;
                prevDataId = currDataId;
                currDataId = get(curr, OFFSET_DATA);
                
                if (prev + ORDER_SIZE != curr)
                {
                    ds.jumpKeyCount++;
                }
                
                if (abs(prevDataId) + DATA_SIZE != abs(currDataId))
                {
                    ds.jumpValCount++;
                }
                
                prevKey = key;
                key = iter.getKey();
                
                V val = iter.getValue();
                if (val == nullptr)
                {
                    ds.nulItemsCount++;
                }
                
                ds.itemCount++;
                
                if (prevKey != nullptr)
                {
                    if (key(prevKey) == 0)
                    {
                        ds.duplicatesCount++;
                    }
                    else if (val == nullptr)
                    {
                        ds.removedItems++;
                    }
                }
                
                iter++;
            }
        }
        
        virtual void finishSerialAllocation()
        {
            orderIndex.store(orderIndexSerial);
            dataIndex.store(dataIndexSerial);
        }
        
        virtual shared_ptr<ItemsIterator> itemsIterator()
        {
            // TODO: Understand if it's legitimate to return a local variable (is the copy contruct performed?)
//			return ItemsIterator(*this);
            return make_shared<ItemsIterator>(*this);
		}

		virtual shared_ptr<ItemsIterator> itemsIterator(int oi)
		{
            // TODO: Understand if it's legitimate to return a local variable (is the copy contruct performed?)
//            ItemsIterator iter (*this);
//            iter.current = oi;
//            return iter;
            
            return make_shared<ItemsIterator>(*this, oi);
		}

        /** Gets the data for the given item, or 'null' if it doesn't exist */
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
			shared_ptr<Rebalancer<K, V, Comparer>> r = getRebalancer();
			if (r == nullptr)
			{
				return false;
			}

			return r->isCompacted(); // TODO: isCompacted  is Rebalancer's method - problema..

		}
        
	protected:
        /** Gets the field of specified offset for given item */
		virtual int get(int item, int offset)
		{
			return orderArray[item + offset];
		}
        
        /** Gets the field of specified offset to 'value' for given item */
		virtual void set(int item, int offset, int value)
		{
			orderArray[item + offset] = value;
		}

	private:
        /** Performs CAS from 'expected' to 'value' for field at specified offset of given item */
		bool cas(int item, int offset, int expected, int value)
		{
            // TODO: replace it with atomic_compare_exchange_strong()
			//return unsafe->compareAndSwapInt(orderArray, Unsafe::ARRAY_INT_BASE_OFFSET + (item + offset) * Unsafe::ARRAY_INT_INDEX_SCALE, expected, value);
            
            return true;
		}

        /** 
         * Binary search for largest-entry smaller than 'key' in sorted part of order-array.
         * @return the index of the entry from which to start a linear search -
         * if key is found, its previous entry is returned! 
         */
		int binaryFind(K key)
		{
			// if there are no sorted keys,or the first item is already larger than key -
			// return the head node for a regular linear search
            if ((sortedCount == 0) || (compare(readKey(FIRST_ITEM), key) >= 0))
			{
				return HEAD_NODE;
			}

			// TODO: check last key to avoid binary search?

			int start = 0;
			int end = sortedCount;

			while (end - start > 1)
			{
				int curr = start + (end - start) / 2;
                if (compare(readKey(curr * ORDER_SIZE + FIRST_ITEM), key) >= 0)
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

	public:
        /**
         * Engages the chunk to a rebalancer r.
         *
         * @param r a rebalancer to engage with
         * @return rebalancer engaged with the chunk
         */
		virtual shared_ptr<Rebalancer<K, V, Comparer>> engage(const shared_ptr<Rebalancer<K, V, Comparer>>& r)
		{
            rebalancer.compare_exchange_strong(nullptr, r);
            return rebalancer.load();
		}

        /**
         * Checks whether the chunk is engaged with a given rebalancer.
         * @param r a rebalancer object. If r is null, verifies that the chunk is not engaged to any rebalancer
         * @return true if the chunk is engaged with r, false otherwise
         */
		virtual bool isEngaged(const shared_ptr<Rebalancer<K, V, Comparer>>& r)
		{
			return rebalancer.load() == r;
		}

        /**
         * Fetches a rebalancer engaged with the chunk.
         * @return rebalancer object or null if not engaged.
         */
		virtual shared_ptr<Rebalancer<K, V, Comparer>> getRebalancer()
		{
			return rebalancer.load();
		}

        /**
         * @return statistics object containing approximate utilization information.
         */
		virtual Statistics& getStatistics()
		{
			return statistics;
		}

        /** 
         * Marks this chunk's next pointer so this chunk is marked as deleted
         * @return the next chunk pointed to once marked (will not change) 
         */
		virtual Chunk<K, V, Comparer>& markAndGetNext()
		{
			// new chunks are ready, we mark frozen chunk's next pointer so it won't change
			// since next pointer can be changed by other split operations we need to do this in a loop - until we succeed
			while (true)
			{
				// if chunk is marked - that is ok and its next pointer will not be changed anymore
				// return whatever chunk is set as next
                MarkableReference<Chunk<K, V, Comparer>> mr_next = next.load();
				if (mr_next.isMarked()) // TODO: Verify it doesn't break the atomicity
				{
					return mr_next.getReference();
				}
				// otherwise try to mark it
				else
				{
					// read chunk's current next
					Chunk<K, V, Comparer>* savedNext = mr_next.getReference();

					// try to mark next while keeping the same next chunk - using CAS
					// if we succeeded then the next pointer we remembered is set and will not change - return it
					if (next.compare_exchange_strong(mr_next, MarkableReference<Chunk<K, V, Comparer>>(savedNext, true)))
					{
						return savedNext;
					}
				}
			}
		}

        /** 
         * Freezes chunk so no more changes can be done in it. also marks pending items as frozen
         */
		virtual void freeze()
		{
			// prevent new puts to the chunk
			orderIndex.fetch_add(orderArray.size());

			// go over thread data of all threads
			for (int i = 0; i < MAX_THREADS; ++i)
			{
				// make sure data is for a Put operatio
				shared_ptr<PutData> currPut = putArray[pad(i)];
				if (currPut == nullptr)
				{
					continue;
				}

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

        /** 
         * Finds and returns the index of the first item that is equal or larger-than the given min key
         * with max version that is equal or less-than given version.
         * returns NONE if no such key exists */
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
                if (compare(key, minKey) >= 0)
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

        /** returns the index of the first item in this chunk with a version <= version */
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

        /** finds and returns the value for the given key, or 'null' if no such key exists */
		virtual V* find(K key, const shared_ptr<PutData>& item)
		{
			// binary search sorted part of order-array to quickly find node to start search at
			// it finds previous-to-key so start with its next
			int curr = get(binaryFind(key), OFFSET_NEXT);

			// iterate until end of list (or key is found)
			while (curr != NONE)
			{
				// compare current item's key to searched key
                int cmp = compare(readKey(curr), key);

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
		V chooseNewer(int item, const shared_ptr<PutData>& pd)
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
				return getData(max(item, pd->orderIndex));
			}
		}

        /** 
         * Adds the given item (allocated in this chunk) to the chunk's linked list
         * @param orderIndex index of item in order-array
         * @param key given for convenience 
         */
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
                    int cmp = compare(readKey(curr), key);
                    cmp = readKey(curr); //(key);

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

							while ((abs(newDataIdx) > abs(oldDataIdx)) && !cas(curr,OFFSET_DATA,oldDataIdx, newDataIdx))
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
							statistics.incrementDuplicates();
						}

						break;
					}
				}
			}
		}

	private:
		bool casData(int curr, void *currData, void *data)
		{
            // TODO: replace it with atomic_compare_exchange_strong()
//			return unsafe->compareAndSwapObject(dataArray, Unsafe::ARRAY_OBJECT_BASE_OFFSET + curr * Unsafe::ARRAY_OBJECT_INDEX_SCALE, currData, data);
            
            return true;

		}

	public:
        /***
         * Appends a new item to the end of items array. The function assumes that the array is sorted in ascending order by (Key, -Version)
         * The method is not thread safe!!!  Should be called for  chunks accessible by single thread only.
         *
         * @param key the key of the new item
         * @param value the value of the new item
         * @param version the version of the new item
         */
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
			return orderIndex.load() / ORDER_SIZE;
		}

		int getNumOfItemsSerial()
		{
			return orderIndexSerial / ORDER_SIZE;
		}

        /***
         * Copies items from srcChunk performing compaction on the fly.
         * @param srcChunk -- chunk to copy from
         * @param oi -- start position for copying
         * @param maxCapacity -- max number of items "this" chunk can contain after copy
         * @return order index of next to the last copied item, NONE if all items were copied
         */
        // TODO: Check if we can declare srcChunk as Chunk<>&
		int copyPart(const shared_ptr<Chunk<K, V, Comparer>>& srcChunk, int oi, int maxCapacity, ScanIndex<K>& scanIndex)
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
			int prevKey = numeric_limits<int>::min();

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
				if (itemsToCopy > 0)
				{
					for (int i = 0; i < itemsToCopy; ++i)
					{
						int offset = i * ORDER_SIZE;
						int oIdx = orderIndexSerial;

						// next should point to the next item
						orderArray[oIdx + offset + OFFSET_NEXT] = oIdx + offset + ORDER_SIZE;
						orderArray[oIdx + offset + OFFSET_VERSION] = abs(srcChunk->orderArray[orderStart + offset + OFFSET_VERSION]);
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
                        // TODO: Just be sure it's correct
                        copy(srcChunk->dataArray[dataIdx], srcChunk->dataArray[dataIdx+itemsToCopy-1], dataArray[dataIndexSerial]);
					}

					dataIndexSerial = dataIndexSerial + itemsToCopy;
				}

				scanIndex.reset(currKey);
				//first item already copied or null
				scanIndex.savedVersion(NONE);

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
						if (scanIndex.shouldKeep(currVersion))
						{
							if (currDataId < 0)
							{
								removedVersion = currVersion;
								scanIndex.savedVersion(currVersion);
							}
							else if (currVersion != removedVersion)
							{
								if (removedVersion != NONE)
								{
									appendItem(currKey, nullptr, removedVersion);
									set(orderIndexSerial - ORDER_SIZE, OFFSET_NEXT, orderIndexSerial);
									scanIndex.savedVersion(removedVersion);
									removedVersion = NONE;
								}


								appendItem(currKey, static_cast<V>(srcChunk->dataArray[currDataId]), currVersion);
								set(orderIndexSerial - ORDER_SIZE, OFFSET_NEXT, orderIndexSerial);
								scanIndex.savedVersion(currVersion);
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

			orderIndex.store(orderIndexSerial);
			dataIndex.store(dataIndexSerial);
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
			int oi = orderIndex.fetch_add(ORDER_SIZE);
			if (oi + ORDER_SIZE > orderArray.size())
			{
				return -1;
			}

			// increment data array to get new index in it
			int di = dataIndex.fetch_add(1);
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

	public:
        /** Gets the current version of the given order-item */
		virtual int getVersion(int orderIndex)
		{
			return abs(get(orderIndex, OFFSET_VERSION));
		}
        
        /** Tries to set (CAS) the version of order-item to specified version
         * @return whatever version is successfuly set (by this thread or another)	 */
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
					if (compare(readKey(curr), readKey(prev)) != 0)
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
                    if (compare(readKey(curr), readKey(prev)) == 0)
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
			cout << this->minKey << " :: ";
			int curr = get(HEAD_NODE, OFFSET_NEXT);

			while (curr != NONE)
			{
				cout << "(" << readKey(curr) << "," << getData(curr) << L"," << curr << L") ";
				curr = get(curr, OFFSET_NEXT);
			}
		}
	};

}

#endif /* Chunk_h */
