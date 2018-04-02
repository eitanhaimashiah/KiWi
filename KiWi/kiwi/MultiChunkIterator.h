#include <vector>
#include <stdexcept>
#include <type_traits>

namespace kiwi
{


	/// <summary>
	/// Created by dbasin on 11/30/15.
	/// Iterator used to iterate over items of multiple chunks.
	/// </summary>
	template<typename K, typename V>
	class MultiChunkIterator
	{
		static_assert(std::is_base_of<Comparable<? super K>, K>::value, L"K must inherit from Comparable<? super K>");

	private:
		Chunk<K, V> *first;
		Chunk<K, V> *last;
		Chunk<K, V> *current;
		Chunk<K, V>::ItemsIterator *iterCurrItem;
		bool hasNextInChunk = false;

	public:
		virtual ~MultiChunkIterator()
		{
			delete first;
			delete last;
			delete current;
			delete iterCurrItem;
		}

	private:
		MultiChunkIterator()
		{
		}

		/// <summary>
		///*
		/// </summary>
		/// <param name="chunks"> - Range of chunks to be iterated </param>
	public:
		MultiChunkIterator(std::vector<Chunk<K, V>*> &chunks)
		{
			if (chunks.empty() || chunks.empty())
			{
				throw std::invalid_argument("Iterator should have at least one item");
			}
			first = chunks[0];
			last = chunks[chunks.size() - 1];
			current = first;
			iterCurrItem = current->itemsIterator();
			hasNextInChunk = iterCurrItem->hasNext();
		}

		MultiChunkIterator(int oi, std::vector<Chunk<K, V>*> &chunks)
		{
			if (chunks.empty() || chunks.empty())
			{
				throw std::invalid_argument("Iterator should have at least one item");
			}
			first = chunks[0];
			last = chunks[chunks.size() - 1];
			current = first;

			iterCurrItem = current->itemsIterator(oi);
			hasNextInChunk = iterCurrItem->hasNext();
		}


		virtual bool hasNext()
		{
			if (iterCurrItem->hasNext())
			{
				return true;
			}

			// cache here the information to improve next()'s performance
			hasNextInChunk = false;

			Chunk<K, V> *nonEmpty = findNextNonEmptyChunk();
			if (nonEmpty == nullptr)
			{
				return false;
			}

			return true;
		}

	private:
		Chunk<K, V> *findNextNonEmptyChunk()
		{
			if (current == last)
			{
				return nullptr;
			}

			Chunk<K, V> *chunk = current->next.getReference();
			if (chunk == nullptr)
			{
				return nullptr;
			}

			while (chunk != nullptr)
			{
				Chunk::ItemsIterator *iter = chunk->itemsIterator();
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

		/// <summary>
		/// After next() iterator points to some item.
		/// The item's Key, Value and Version can be fetched by corresponding getters.
		/// </summary>
	public:
		virtual void next()
		{
			if (hasNextInChunk)
			{
				iterCurrItem->next();
				return;
			}

			Chunk<K, V> *nonEmpty = findNextNonEmptyChunk();

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

		/// <summary>
		///*
		/// Fetches VersionsIterator to iterate versions of current key.
		/// Will help to separate versions and keys data structures in future optimizations. </summary>
		/// <returns> VersionsIterator object </returns>
		virtual Chunk::ItemsIterator::VersionsIterator *versionsIterator()
		{
			return iterCurrItem->versionsIterator();
		}

		/// <summary>
		///*
		/// </summary>
		/// <returns> A copy  with the same state. </returns>
		virtual MultiChunkIterator<K, V> *cloneIterator()
		{
			MultiChunkIterator<K, V> *newIterator = new MultiChunkIterator<K, V>();
			newIterator->first = this->first;
			newIterator->last = this->last;
			newIterator->current = this->current;
			newIterator->hasNextInChunk = this->hasNextInChunk;

			newIterator->iterCurrItem = iterCurrItem->cloneIterator();

			return newIterator;
		}
	};

}