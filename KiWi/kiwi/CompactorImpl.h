#ifndef CompactorImpl_h
#define CompactorImpl_h

#include "Compactor.h"
#include <vector>
#include <list>
#include <type_traits>

using namespace std;

namespace kiwi
{
	template<typename K, typename V, class Comparer = less<K>>
	class CompactorImpl : public Compactor<K, V, Comparer>
	{
	private:
		const int LOW_THRESHOLD = Chunk<K, V, Comparer>::MAX_ITEMS / 2;
		const int HIGH_THRESHOLD = Chunk<K, V, Comparer>::MAX_ITEMS - KiWi::MAX_THREADS; // max number of pending scan versions
		const int MAX_RANGE_TO_APPEND = static_cast<int>(0.2*Chunk<K, V, Comparer>::MAX_ITEMS);
		Chunk<K, V, Comparer> *lastCheckedForAppend = nullptr;



	public:
		virtual ~CompactorImpl()
		{
			delete lastCheckedForAppend;
		}

		virtual vector<Chunk<K, V, Comparer>*> compact(vector<Chunk<K, V, Comparer>*> &frozenChunks, ScanIndex<K> *scanIndex)
		{
			vector<Chunk<K, V, Comparer>*>::const_iterator iterFrozen = frozenChunks.begin();

//JAVA TO C++ CONVERTER TODO TASK: Java iterators are only converted within the context of 'while' and 'for' loops:
			Chunk<K, V, Comparer> *firstFrozen = iterFrozen.next();
			Chunk<K, V, Comparer> *currFrozen = firstFrozen;
			Chunk<K, V, Comparer> *currCompacted = firstFrozen->newChunk(firstFrozen->minKey);

			int oi = firstFrozen->getFirstItemOrderId();

			vector<Chunk<K, V, Comparer>*> compacted = list<Chunk<K, V, Comparer>*>();

			while (true)
			{
				oi = currCompacted->copyPart(currFrozen, oi, LOW_THRESHOLD, scanIndex);

				// if completed reading curr freezed chunk
				if (oi == Chunk<K, V, Comparer>::NONE)
				{
//JAVA TO C++ CONVERTER TODO TASK: Java iterators are only converted within the context of 'while' and 'for' loops:
					if (!iterFrozen.hasNext())
					{
						break;
					}

//JAVA TO C++ CONVERTER TODO TASK: Java iterators are only converted within the context of 'while' and 'for' loops:
					currFrozen = iterFrozen.next();
					oi = currFrozen->getFirstItemOrderId();
				}
				else // filled compacted chunk up to LOW_THRESHOLD
				{
					vector<Chunk<K, V, Comparer>*> frozenSuffix = frozenChunks.subList(iterFrozen.previousIndex(), frozenChunks.size());

					// try to look ahead and add frozen suffix
					if (canAppendSuffix(oi, frozenSuffix, MAX_RANGE_TO_APPEND))
					{
						completeCopy(currCompacted, oi, frozenSuffix, scanIndex);
						break;

					}
					else
					{
						Chunk<K, V, Comparer> *c = firstFrozen->newChunk(currFrozen->readKey(oi));
						currCompacted->next.set(c,false);

						compacted.push_back(currCompacted);
						currCompacted = c;
					}
				}

			}

			compacted.push_back(currCompacted);

			return compacted;
		}

	private:
		bool canAppendSuffix(int oi, vector<Chunk<K, V, Comparer>*> &frozenSuffix, int maxCount)
		{
			MultiChunkIterator *iter = new MultiChunkIterator(oi, frozenSuffix);
			int counter = 1;

			while (iter->hasNext() && counter < maxCount)
			{
				iter->next();
				counter++;
			}

			return counter < maxCount;
		}

		void completeCopy(Chunk<K, V, Comparer> *dest, int oi, vector<Chunk<K, V, Comparer>*> &srcChunks, ScanIndex<K> *scanIndex)
		{
			vector<Chunk<K, V, Comparer>*>::const_iterator iter = srcChunks.begin();
//JAVA TO C++ CONVERTER TODO TASK: Java iterators are only converted within the context of 'while' and 'for' loops:
			Chunk<K, V, Comparer> *src = iter.next();
			dest->copyPart(src,oi, Chunk<K, V, Comparer>::MAX_ITEMS, scanIndex);

			while (iter != srcChunks.end())
			{

				src = *iter;
				oi = src->getFirstItemOrderId();
				dest->copyPart(src,oi, Chunk<K, V, Comparer>::MAX_ITEMS, scanIndex);
				iter++;
			}
		}

	};
}

#endif /* CompactorImpl_h */
