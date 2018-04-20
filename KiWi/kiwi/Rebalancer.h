#ifndef Rebalancer_h
#define Rebalancer_h

#include <vector>
#include <list>
#include <cmath>
#include <stdexcept>

#include "CompactorImpl.h"
#include "exceptionhelper.h"
#include "ScanIndex.h"
#include "Chunk.h"
#include "KiWi.h"
#include "MarkableReference.h"


using namespace std;

namespace kiwi
{
	template<typename K, typename V, class Comparer = less<K>>
	class Rebalancer
	{
	public:
        static constexpr double MAX_AFTER_MERGE_PART = 0.5;

    /******** Policy ******************/
	public:
		class Policy
		{
			virtual Chunk<K, V, Comparer> findNextCandidate() = 0;
			virtual void updateRangeView() = 0;
			virtual Chunk<K, V, Comparer> getFirstChunkInRange() = 0;
			virtual Chunk<K, V, Comparer> getLastChunkInRange() = 0;
		};

	public:
		class PolicyImpl : public Policy
		{
		private:
			Rebalancer<K, V>& outerInstance;

            static int  RebalanceSize;

			int chunksInRange = 0;
			int itemsInRange = 0;

			int maxAfterMergeItems = 0;


		public:
			Chunk<K, V, Comparer> *first;
			Chunk<K, V, Comparer> *last;

			PolicyImpl(Rebalancer<K, V> & rebalancer, Chunk<K, V, Comparer> *startChunk);

            PolicyImpl(Rebalancer<K, V, Comparer>& outerInstance, Chunk<K, V, Comparer> *startChunk) : outerInstance(outerInstance)
            {
                if (startChunk == nullptr)
                {
                    throw invalid_argument("startChunk is null in policy");
                }
                first = startChunk;
                last = startChunk;
                chunksInRange = 1;
                itemsInRange = startChunk->getStatistics().getCompactedCount();
                maxAfterMergeItems = static_cast<int>(Chunk<K, V, Comparer>::MAX_ITEMS * MAX_AFTER_MERGE_PART);
            }
            
		private:
            /**
             * verifies that the chunk is not engaged and not null
             * @param chunk candidate chunk for range extension
             * @return true if not engaged and not null
             */
			bool isCandidate(Chunk<K, V, Comparer> *chunk)
            {
                // do not take chunks that are engaged with another rebalancer or infant
                if (chunk == nullptr || !chunk->isEngaged(nullptr) || chunk->isInfant())
                {
                    return false;
                }
                return true;
            }
            
		public:
			Chunk<K, V, Comparer> *findNextCandidate() override
            {
                updateRangeView();
                
                // allow up to RebalanceSize chunks to be engaged
                if (chunksInRange >= RebalanceSize)
                {
                    return nullptr;
                }
                
                Chunk<K, V, Comparer> *next = outerInstance->chunkIterator->getNext(last);
                Chunk<K, V, Comparer> *prev = outerInstance->chunkIterator->getPrev(first);
                Chunk<K, V, Comparer> *candidate = nullptr;
                
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
            
            void updateRangeView() override
            {
                
                updateRangeFwd();
                updateRangeBwd();
                
            }
            
            Chunk<K, V, Comparer> *getFirstChunkInRange() override
            {
                return first;
            }

			Chunk<K, V, Comparer> *getLastChunkInRange() override
            {
                return last;
            }

		private:
			void addToCounters(Chunk<K, V, Comparer> *chunk)
            {
                itemsInRange += chunk->getStatistics().getCompactedCount();
                chunksInRange++;
            }
            
			void updateRangeFwd()
            {
                
                while (true)
                {
                    Chunk<K, V, Comparer> *next = outerInstance->chunkIterator->getNext(last);
                    if (next == nullptr || !next->isEngaged(outerInstance))
                    {
                        break;
                    }
                    last = next;
                    addToCounters(last);
                }
            }
            
            void updateRangeBwd()
            {
                while (true)
                {
                    Chunk<K, V, Comparer> *prev = outerInstance->chunkIterator->getPrev(first);
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
        };
        
    public:
		virtual ~Rebalancer()
		{
			delete nextToEngage;
			delete startChunk;
			delete chunkIterator;
			delete compactedChunks;
			delete engagedChunks;
			delete freezedItems;
		}

		virtual Policy *createPolicy(Chunk<K, V, Comparer> *startChunk)
		{
			return new PolicyImpl(this, startChunk);
		}

		/// <summary>
		///****** Members *********** </summary>


	private:
		atomic<MarkableReference<Chunk<K, V, Comparer>*>> *nextToEngage;
		Chunk<K, V, Comparer> *startChunk;
		ChunkIterator<K, V> *chunkIterator;

        // TODO: this type is absolutely wrong
        atomic<MarkableReference<vector<Chunk<K, V, Comparer>*>>> compactedChunks; //= new AtomicReference<vector<Chunk<K, V, Comparer>*>>(nullptr);
        atomic<MarkableReference<vector<Chunk<K, V, Comparer>*>>> engagedChunks; // = new AtomicReference<vector<Chunk<K, V, Comparer>*>>(nullptr);
        atomic<bool> freezedItems;// = new atomic<bool>(false);


	   /// <summary>
	   ///***** Constructors ******** </summary>

	public:
		Rebalancer(Chunk<K, V, Comparer> *chunk, ChunkIterator<K, V> *chunkIterator)
		{
			if (chunk == nullptr || chunkIterator == nullptr)
			{
				throw invalid_argument("Rebalancer construction with null args");
			}

			nextToEngage = new atomic<MarkableReference<vector<Chunk<K, V, Comparer>*>>>(chunk);
			this->startChunk = chunk;
			this->chunkIterator = chunkIterator;
		}

		/// <summary>
		///***** Public methods ********* </summary>

		// assumption -- chunk once engaged remains with the same rebalance object forever, till GC
		virtual Rebalancer<K, V> *engageChunks()
		{
			// the policy object will store first, last refs of engaged range
			Policy *p = createPolicy(startChunk);

			while (true)
			{
				Chunk<K, V, Comparer> *next = nextToEngage->get();
				if (next == nullptr)
				{
					break;
				}

				next->engage(this);

				if (!next->isEngaged(this) && next == startChunk)
				{
					return next->getRebalancer().engageChunks();
				}

				// policy caches last discovered  interval [first, last] of engaged range
				// to get next candidate policy traverses from first backward,
				//  from last forward to find non-engaged chunks connected to the engaged interval
				// if we return null here the policy decided to terminate the engagement loop

				Chunk<K, V, Comparer> *candidate = p->findNextCandidate();

				// if fail to CAS here, another thread has updated next candidate
				// continue to while loop and try to engage it
				nextToEngage->compareAndSet(next, candidate);
			}

			p->updateRangeView();
			vector<Chunk<K, V, Comparer>*> engaged = createEngagedList(p->getFirstChunkInRange());

			if (engagedChunks->compareAndSet(nullptr,engaged) && Parameters::countCompactions)
			{
				Parameters::compactionsNum.fetch_add(1); // if CAS fails here - another thread has updated it
				Parameters::engagedChunks.fetch_add(engaged.size());
			}

		   return this;
		}


		/// <summary>
		///*
		/// Freeze the engaged chunks. Should be called after engageChunks.
		/// Marks chunks as freezed, prevents future updates of the engagead chunks </summary>
		/// <returns> total number of items in the freezed range </returns>
		virtual Rebalancer *freeze()
		{
			if (isFreezed())
			{
				return this;
			}


			for (auto chunk : getEngagedChunks())
			{
				chunk->freeze();
			}

			freezedItems.store(true);

			return this;
		}


		virtual Rebalancer *compact(ScanIndex<K> *scanIndex)
		{
			if (isCompacted())
			{
				return this;
			}

			Compactor *c = new CompactorImpl();
			vector<Chunk<K, V, Comparer>*> compacted = c->compact(getEngagedChunks(),scanIndex);

			// if fail here, another thread succeeded
			compactedChunks->compareAndSet(nullptr,compacted);

			return this;
		}

		virtual bool isCompacted()
		{
			return compactedChunks->get() != nullptr;
		}

		virtual bool isFreezed()
		{
			return freezedItems.load();
		}

		virtual bool isEngaged()
		{
			return engagedChunks->get() != nullptr;
		}

		virtual vector<Chunk<K, V, Comparer>*> getCompactedChunks()
		{
			if (!isCompacted())
			{
				throw IllegalStateException("Trying to get compacted chunks before compaction stage completed");
			}

			return compactedChunks->get();
		}

		virtual vector<Chunk<K, V, Comparer>*> getEngagedChunks()
		{
			vector<Chunk<K, V, Comparer>*> engaged = engagedChunks->get();
			if (engaged.empty())
			{
				throw IllegalStateException("Trying to get engaged before engagement stage completed");
			}

			return engaged;
		}


	private:
		vector<Chunk<K, V, Comparer>*> createEngagedList(Chunk<K, V, Comparer> *firstChunkInRange)
		{
			Chunk<K, V, Comparer> *current = firstChunkInRange;
			vector<Chunk<K, V, Comparer>*> engaged = list<Chunk<K, V, Comparer>*>();

			while (current != nullptr && current->isEngaged(this))
			{
				engaged.push_back(current);
				current = current->next.getReference();
			}

			if (engaged.empty())
			{
				throw IllegalStateException("Engaged list cannot be empty");
			}

			return engaged;
		}


	};
    
    template<typename K, typename V, class Comparer>
    int  Rebalancer<K, V, Comparer>::RebalanceSize = KiWi<K, V, Comparer>::RebalanceSize;

}

#endif /* Rebalancer_h */
