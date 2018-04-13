#ifndef ScanIterator_h
#define ScanIterator_h

#include "exceptionhelper.h"
#include "PutData.h"

namespace kiwi
{
	template<typename K, typename V>
	class ScanIterator : public iterator<V>
	{
	private:
		const K maxKey; // max key (inclusive) for this scan - beyond it the iterator is finished
		const int version; // version for this scan- larger versions are ignored
		SortedMap<K, PutData<K, V>*> *const items; // items map - for items that are currently added (from thread-array)
		Iterator<K> *const iter; // iterator over items map keys

		Chunk<K, V, Comparer> *chunk; // current chunk, or 'null' if no more items from chunks

		K keyItems; // key of current item (next to be returned) in items map
		K keyChunk; // key of current item from the chunks
		int idxChunk = 0; // index of current item in current chunk
		V nextVal; // next value that should be returned, null if done

	public:
		virtual ~ScanIterator()
		{
			delete items;
			delete iter;
			delete chunk;
		}

		ScanIterator(K min, K max, int version, Chunk<K, V, Comparer> *chunk, SortedMap<K, PutData<K, V>*> *items) : maxKey(max), version(version), items(items), iter(items->keySet().begin())
		{

			this->chunk = chunk;

			// find first items in the data structure (chunks)
			// get the first index equal or larger-than minKey (matching version)
			idxChunk = chunk->findFirst(min, version);
			handleChunksItem();

			// find first item in the thread-array (items map)
			// the items map is a sorted map, so just get the first item-- if it exists
			if (iter->hasNext())
			{
				keyItems = iter->next();
			}

			// update next value to hold first actual (non-deleted) value
			updateNextValue();
		}

		/// <summary>
		/// handles current chunk item - proceeding to next chunk if needed (item is NONE), and updating keyChunks </summary>
	private:
		void handleChunksItem()
		{
			while (idxChunk == Chunk::NONE)
			{
				// proceed to next chunk
				chunk = chunk->next.getReference();

				// if no next chunk - we're done
				if (chunk == nullptr)
				{
					break;
				}
				// otherwise check first item in chunk
				else
				{
					// if chunk's min key isn't too large - find matching item in it
					if (chunk->minKey->compareTo(maxKey) <= 0)
					{
						idxChunk = chunk->getFirst(version);
					}
					// otherwise we're done, nullify chunk variable
					else
					{
						chunk = nullptr;
						break;

					}
				}
			}

			// if some item was found, update chunk key variable for "merge-sort" in next() method
			if (chunk != nullptr)
			{
				keyChunk = chunk->readKey(idxChunk);

				// make sure we didn't exceed max key
				if (keyChunk(maxKey) > 0)
				{
					chunk = nullptr;
				}
			}
		}

		bool hasNextValue()
		{
			return ((chunk != nullptr) || (keyItems != nullptr));
		}

	public:
		bool hasNext() override
		{
			return (nextVal != nullptr);
		}

	private:
		void updateNextValue()
		{
			// reset next value to null, to start searching for an actual (non-deleted) next value
			nextVal = nullptr;

			// find next value, while skipping null (i.e., deleted) items/values
			while ((nextVal == nullptr) && (hasNextValue()))
			{
				nextVal = getNextValue();
			}
		}

	public:
		V next() override
		{
			if (nextVal == nullptr)
			{
				throw NoSuchElementException();
			}

			// set to return our next-value
			V retVal = nextVal;

			// update next value to actual (non-deleted) next value
			updateNextValue();

			// return previously-stored value
			return retVal;
		}

	private:
		V getNextValue()
		{
			/*
			// "merge-sort" chunk items and sortedmap items
			// no items (hasNext() is false), so this is an error
			if ((chunk == null) && (keyItems == null))
			{
				return null;
			}
			// items map finished - return from chunks
			else if (keyItems == null)
			{
				return nextFromChunks();
			}
			// chunks items finished - return from items map
			else if (chunk == null)
			{
				return nextFromMap();
			}
			// both have items - compare keys
			else
			{
				int cmp = keyChunk.compareTo(keyItems);
				
				// chunks key is smaller
				if (cmp < 0)
					return nextFromChunks();
				// items map key is smaller
				if (cmp > 0)
					return nextFromMap();
				
				// keys are equal - check versions
				PutData<K,V> pd = items.get(keyItems);
				int verChunk = chunk.getVersion(idxChunk);
				int verItems = pd.chunk.getVersion(pd.orderIndex);
				
				// we move both next since they're the same item - both need to proceed and only one should be returned
				V valChunk = nextFromChunks();
				V valItems = nextFromMap();
				
				// chunk's version is newer
				if (verChunk > verItems)
					return valChunk;
				// items map version is newer
				else if (verChunk < verItems)
					return valItems;
				
				// if PutData's chunk is different- then chunks item is newer
				if (pd.chunk != chunk)
					return valChunk;
				
				// otherwise in same chunk - decide according to item index
				if (idxChunk > pd.orderIndex)
					return valChunk;
				else
					return valItems;
			}
			*/
			throw NotImplementedException();
		}

		V nextFromMap()
		{
			/*
			PutData<K,V> pd = items.get(keyItems);
	
			while (true)
			{
				if (iter.hasNext())
				{
					K oldKey = keyItems;
					keyItems = iter.next();
					
					if (oldKey.compareTo(keyItems) != 0)
						break;
				}
				else
				{
					keyItems = null;
					break;
				}
			}
			
			return pd.chunk.getData(pd.orderIndex);
			*/
			throw NotImplementedException();
		}
		V nextFromChunks()
		{
			V val = chunk->getData(idxChunk);

			// if last key was max, no need to search further
			if (keyChunk(maxKey) == 0)
			{
				chunk = nullptr;
			}
			// otherwise find next key in chunks and handle it
			else
			{
				idxChunk = chunk->findNext(idxChunk, version, keyChunk);
				handleChunksItem();
			}

			return val;
		}

	public:
		void remove() override
		{
			throw NotImplementedException();
		}
	};

}

#endif /* ScanIterator_h */

