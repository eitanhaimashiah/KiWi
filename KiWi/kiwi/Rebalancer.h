#include "ScanIndex.h"
#include <vector>
#include <list>
#include <cmath>
#include <stdexcept>
#include "exceptionhelper.h"
#include <type_traits>

namespace kiwi
{


	/// <summary>
	/// Created by dbasin on 11/24/15.
	/// </summary>
	template<typename K, typename V>
	class Rebalancer
	{
		static_assert(std::is_base_of<Comparable<? super K>, K>::value, L"K must inherit from Comparable<? super K>");


	public:
//JAVA TO C++ CONVERTER TODO TASK: Native C++ does not allow initialization of static non-const/integral fields in their declarations - choose the conversion option for separate .h and .cpp files:
		static double MAX_AFTER_MERGE_PART = 0.5;

		/// <summary>
		///******** Policy ****************** </summary>
	public:
		class Policy
		{
		private:
			Rebalancer<K*, V*> *outerInstance;

		public:
			virtual ~Policy()
			{
				delete outerInstance;
			}

			Policy(Rebalancer<K, V> *outerInstance);

			virtual Chunk<K*, V*> *findNextCandidate() = 0;
			virtual void updateRangeView() = 0;
			virtual Chunk<K*, V*> *getFirstChunkInRange() = 0;
			virtual Chunk<K*, V*> *getLastChunkInRange() = 0;
		};

	public:
		class PolicyImpl : public Policy
		{
		private:
			Rebalancer<K*, V*> *outerInstance;

			const int RebalanceSize = KiWi::RebalanceSize;



			int chunksInRange = 0;
			int itemsInRange = 0;

			int maxAfterMergeItems = 0;


		public:
			Chunk<K*, V*> *first;
			Chunk<K*, V*> *last;

			virtual ~PolicyImpl()
			{
				delete first;
				delete last;
				delete outerInstance;
			}

			PolicyImpl(Rebalancer<K, V> *outerInstance, Chunk<K*, V*> *startChunk);

			/// <summary>
			///*
			/// verifies that the chunk is not engaged and not null </summary>
			/// <param name="chunk"> candidate chunk for range extension </param>
			/// <returns> true if not engaged and not null </returns>
		private:
			bool isCandidate(Chunk<K*, V*> *chunk);

			/// <summary>
			///*
			/// 
			/// @return
			/// </summary>
		public:
			Chunk<K*, V*> *findNextCandidate() override;

			void updateRangeView() override;

			Chunk<K*, V*> *getFirstChunkInRange() override;

			Chunk<K*, V*> *getLastChunkInRange() override;

		private:
			void addToCounters(Chunk<K*, V*> *chunk);

			void updateRangeFwd();

			void updateRangeBwd();
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

		virtual Policy *createPolicy(Chunk<K, V> *startChunk)
		{
			return new PolicyImpl(this, startChunk);
		}

		/// <summary>
		///****** Members *********** </summary>


	private:
		AtomicReference<Chunk<K, V>*> *nextToEngage;
		Chunk<K, V> *startChunk;
		ChunkIterator<K, V> *chunkIterator;

		AtomicReference<std::vector<Chunk<K, V>*>> *compactedChunks = new AtomicReference<std::vector<Chunk<K, V>*>>(nullptr);
		AtomicReference<std::vector<Chunk<K, V>*>> *engagedChunks = new AtomicReference<std::vector<Chunk<K, V>*>>(nullptr);
		AtomicBoolean *freezedItems = new AtomicBoolean(false);


	   /// <summary>
	   ///***** Constructors ******** </summary>

	public:
		Rebalancer(Chunk<K, V> *chunk, ChunkIterator<K, V> *chunkIterator)
		{
			if (chunk == nullptr || chunkIterator == nullptr)
			{
				throw std::invalid_argument("Rebalancer construction with null args");
			}

			nextToEngage = new AtomicReference<Chunk<K,V>>(chunk);
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
				Chunk<K, V> *next = nextToEngage->get();
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

				Chunk *candidate = p->findNextCandidate();

				// if fail to CAS here, another thread has updated next candidate
				// continue to while loop and try to engage it
				nextToEngage->compareAndSet(next, candidate);
			}

			p->updateRangeView();
			std::vector<Chunk<K, V>*> engaged = createEngagedList(p->getFirstChunkInRange());

			if (engagedChunks->compareAndSet(nullptr,engaged) && Parameters::countCompactions)
			{
				Parameters::compactionsNum::getAndIncrement(); // if CAS fails here - another thread has updated it
				Parameters::engagedChunks::addAndGet(engaged.size());
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

			freezedItems->set(true);

			return this;
		}


		virtual Rebalancer *compact(ScanIndex<K> *scanIndex)
		{
			if (isCompacted())
			{
				return this;
			}

			Compactor *c = new CompactorImpl();
			std::vector<Chunk<K, V>*> compacted = c->compact(getEngagedChunks(),scanIndex);

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
			return freezedItems->get();
		}

		virtual bool isEngaged()
		{
			return engagedChunks->get() != nullptr;
		}

		virtual std::vector<Chunk<K, V>*> getCompactedChunks()
		{
			if (!isCompacted())
			{
				throw IllegalStateException(L"Trying to get compacted chunks before compaction stage completed");
			}

			return compactedChunks->get();
		}

		virtual std::vector<Chunk<K, V>*> getEngagedChunks()
		{
			std::vector<Chunk<K, V>*> engaged = engagedChunks->get();
			if (engaged.empty())
			{
				throw IllegalStateException(L"Trying to get engaged before engagement stage completed");
			}

			return engaged;
		}


	private:
		std::vector<Chunk<K, V>*> createEngagedList(Chunk<K, V> *firstChunkInRange)
		{
			Chunk<K, V> *current = firstChunkInRange;
			std::vector<Chunk<K, V>*> engaged = std::list<Chunk<K, V>*>();

			while (current != nullptr && current->isEngaged(this))
			{
				engaged.push_back(current);
				current = current->next.getReference();
			}

			if (engaged.empty())
			{
				throw IllegalStateException(L"Engaged list cannot be empty");
			}

			return engaged;
		}


	};

}
