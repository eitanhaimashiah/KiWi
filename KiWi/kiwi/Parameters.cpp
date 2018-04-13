#include "Parameters.h"

namespace kiwi
{
    int Parameters::numThreads = 1;
    int Parameters::numMilliseconds = 5000;
    int Parameters::numWrites = 40;
    int Parameters::numWriteAlls = 0;
    int Parameters::numSnapshots = 0;
    int Parameters::range = 2048;
    int Parameters::size = 1024;
    int Parameters::warmUp = 5;
    int Parameters::iterations = 1;
    bool Parameters::detailedStats = false;
    string Parameters::benchClassName = "skiplists.lockfree.NonBlockingFriendlySkipListMap";
    bool Parameters::rangeQueries = false;
    int Parameters::minRangeSize = 10;
    int Parameters::maxRangeSize = 20;
    bool Parameters::countKeysInRange = true;
    double Parameters::rebalanceProbPerc = 2;
    double Parameters::sortedRebalanceRatio = 1.8;
    Parameters::FillType Parameters::fillType = FillType::Random;
    atomic<int> Parameters::compactionsNum;
    atomic<int> Parameters::engagedChunks;
    bool Parameters::countCompactions = false;
}
