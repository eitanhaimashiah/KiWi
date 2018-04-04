

#include "Rebalancer.h"

namespace kiwi
{

	Rebalancer<K, V>::Policy::Policy(Rebalancer<K, V> *outerInstance) : outerInstance(outerInstance)
	{
	}

	Rebalancer<K, V>::PolicyImpl::PolicyImpl(Rebalancer<K, V> *outerInstance, Chunk<K*, V*> *startChunk) : Policy(outerInstance), outerInstance(outerInstance)
	{
		if (startChunk == nullptr)
		{
			throw invalid_argument("startChunk is null in policy");
		}
		first = startChunk;
		last = startChunk;
		chunksInRange = 1;
		itemsInRange = startChunk->getStatistics().getCompactedCount();
		maxAfterMergeItems = static_cast<int>(Chunk::MAX_ITEMS * MAX_AFTER_MERGE_PART);
	}

	bool Rebalancer<K, V>::PolicyImpl::isCandidate(Chunk<K*, V*> *chunk)
	{
		// do not take chunks that are engaged with another rebalancer or infant
		if (chunk == nullptr || !chunk->isEngaged(nullptr) || chunk->isInfant())
		{
			return false;
		}
		return true;
	}

	Chunk<K*, V*> *Rebalancer<K, V>::PolicyImpl::findNextCandidate()
	{

		updateRangeView();

		// allow up to RebalanceSize chunks to be engaged
		if (chunksInRange >= RebalanceSize)
		{
			return nullptr;
		}

		Chunk<K*, V*> *next = outerInstance->chunkIterator->getNext(last);
		Chunk<K*, V*> *prev = outerInstance->chunkIterator->getPrev(first);
		Chunk<K*, V*> *candidate = nullptr;

		if (!isCandidate(next))
		{
			next = nullptr;
		}
		if (!isCandidate(prev))
		{
			prev = nullptr;
		}

		if (next == nullptr && prev == nullptr)
		{
			return nullptr;
		}

		if (next == nullptr)
		{
			candidate = prev;
		}
		else if (prev == nullptr)
		{
			candidate = next;
		}
		else
		{
			candidate = prev->getStatistics().getCompactedCount() < next->getStatistics().getCompactedCount() ? prev : next;
		}


		int newItems = candidate->getStatistics().getCompactedCount();
		int totalItems = itemsInRange + newItems;


		int chunksAfterMerge = static_cast<int>(ceil((static_cast<double>(totalItems)) / maxAfterMergeItems));

		// if the the chosen chunk may reduce the number of chunks -- return it as candidate
		if (chunksAfterMerge < chunksInRange + 1)
		{
			return candidate;
		}
		else
		{
			return nullptr;
		}
	}

	void Rebalancer<K, V>::PolicyImpl::updateRangeView()
	{

		updateRangeFwd();
		updateRangeBwd();

	}

	Chunk<K*, V*> *Rebalancer<K, V>::PolicyImpl::getFirstChunkInRange()
	{
		return first;
	}

	Chunk<K*, V*> *Rebalancer<K, V>::PolicyImpl::getLastChunkInRange()
	{
		return last;
	}

	void Rebalancer<K, V>::PolicyImpl::addToCounters(Chunk<K*, V*> *chunk)
	{
		itemsInRange += chunk->getStatistics().getCompactedCount();
		chunksInRange++;
	}

	void Rebalancer<K, V>::PolicyImpl::updateRangeFwd()
	{

		while (true)
		{
			Chunk<K*, V*> *next = outerInstance->chunkIterator->getNext(last);
			if (next == nullptr || !next->isEngaged(outerInstance))
			{
				break;
			}
			last = next;
			addToCounters(last);
		}
	}

	void Rebalancer<K, V>::PolicyImpl::updateRangeBwd()
	{
		 while (true)
		 {
			 Chunk<K*, V*> *prev = outerInstance->chunkIterator->getPrev(first);
			 if (prev == nullptr || !prev->isEngaged(outerInstance))
			 {
				 break;
			 }
			 // double check here, after we know that prev is engaged, thus cannot be updated
			 if (prev->next.getReference() == first)
			 {
				 first = prev;
				 addToCounters(first);
			 }
		 }
	}
}
