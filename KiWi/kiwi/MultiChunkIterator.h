#ifndef MultiChunkIterator_h
#define MultiChunkIterator_h

#include <vector>
#include <stdexcept>

#include "Chunk.h"

using namespace std;

namespace kiwi
{
	template<typename K, typename V>
	class MultiChunkIterator
	{
        using typename Chunk<K, V, Comparer>::ItemsIterator;
        
	private:
        // TODO: Check if I can change to ref, the current problem is comparing first (of ref type) to current
		Chunk<K, V, Comparer>* first;
		Chunk<K, V, Comparer>* last;
		Chunk<K, V, Comparer>* current;
		shared_ptr<ItemsIterator> iterCurrItem;
		bool hasNextInChunk = false;

        MultiChunkIterator() {}

	public:
        /***
         * @param chunks - Range of chunks to be iterated
         */
		MultiChunkIterator(vector<Chunk<K, V, Comparer>*>& chunks)
		{
			if (chunks.empty())
			{
				throw invalid_argument("Iterator should have at least one item");
			}
			first = chunks[0];
			last = chunks[chunks.size() - 1];
			current = first;
            
			iterCurrItem = current->itemsIterator();
			hasNextInChunk = iterCurrItem->hasNext();
		}

		MultiChunkIterator(int oi, vector<shared_ptr<Chunk<K, V, Comparer>>>& chunks)
		{
			if (chunks.empty())
			{
				throw invalid_argument("Iterator should have at least one item");
			}
            first = chunks[0];
            last = chunks[chunks.size() - 1];
            current = first;
            
			iterCurrItem = current->itemsIterator(oi);
			hasNextInChunk = iterCurrItem->hasNext();
		}
        
        /***
         * Copy constructor
         */
        MultiChunkIterator(const MultiChunkIterator<K, V>& other):
        first(other.first),
        last(other.last),
        current(other.current),
        hasNextInChunk(other.hasNextInChunk),
        iterCurrItem(other.iterCurrItem? (new ItemsIterator(*other.iterCurrItem)) : nullptr)
        {}
        
        virtual bool hasNext()
		{
			if (iterCurrItem->hasNext())
			{
				return true;
			}

			// cache here the information to improve next()'s performance
			hasNextInChunk = false;

			Chunk<K, V, Comparer>* nonEmpty = findNextNonEmptyChunk();
			if (nonEmpty == nullptr)
			{
				return false;
			}

			return true;
		}

	private:
		Chunk<K, V, Comparer>* findNextNonEmptyChunk()
		{
			if (current == last)
			{
				return nullptr;
			}

			Chunk<K, V, Comparer>* chunk = current->next.getReference();
			if (chunk == nullptr)
			{
				return nullptr;
			}

			while (chunk != nullptr)
			{
				auto iter = chunk->itemsIterator();
				if (iter->hasNext())
				{
					return chunk;
				}

				if (chunk == last)
				{
					return nullptr;
				}

				chunk = chunk->next.getReference();
			}

			return nullptr;
		}

	public:
        /**
         * After next() iterator points to some item.
         * The item's Key, Value and Version can be fetched by corresponding getters.
         */
		virtual void next()
		{
			if (hasNextInChunk)
			{
				iterCurrItem->next();
				return;
			}

			Chunk<K, V, Comparer>* nonEmpty = findNextNonEmptyChunk();

			current = nonEmpty;

			iterCurrItem = nonEmpty->itemsIterator();
			iterCurrItem->next();

			hasNextInChunk = iterCurrItem->hasNext();
		}


		virtual K getKey()
		{
			return iterCurrItem->getKey();
		}


		virtual V getValue()
		{
			return iterCurrItem->getValue();
		}


		virtual int getVersion()
		{
			return iterCurrItem->getVersion();
		}

        /**
         * Fetches VersionsIterator to iterate versions of current key.
         * Will help to separate versions and keys data structures in future optimizations.
         * @return VersionsIterator object
         */
		virtual typename Chunk<K, V, Comparer>::ItemsIterator::VersionsIterator* versionsIterator()
		{
			return iterCurrItem->versionsIterator();
		}
	};
}

#endif /* MultiChunkIterator_h */
