#ifndef Parameters_h
#define Parameters_h

#include <string>
#include <memory>

using namespace std;

namespace kiwi
{
    /**
     * Parameters of the C++ version of the Synchrobench benchmark.
     *
     * @author Vincent Gramoli
     */
	class Parameters
	{
	public:
        enum FillType { Random, Ordered, DropHalf, Drop90 };
        
		static int numThreads;
		static int numMilliseconds;
		static int numWrites;
		static int numWriteAlls;
		static int numSnapshots;
		static int range;
		static int size;
		static int warmUp;
		static int iterations;

		static bool detailedStats;

		static string benchClassName;
		static bool rangeQueries;
		static int minRangeSize;
		static int maxRangeSize;

		static bool countKeysInRange;
		static double rebalanceProbPerc;
		static double sortedRebalanceRatio;

		static FillType fillType;

		static atomic<int> compactionsNum;
		static atomic<int> engagedChunks;
		static bool countCompactions;
	};
}

#endif /* Parameters_h */
