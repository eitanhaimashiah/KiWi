#include "Parameters.h"

namespace kiwi
{
	using sun::reflect::generics::reflectiveObjects::NotImplementedException;
	using util::CombinedGenerator;
	using util::IntegerGenerator;
	using util::Pair;
	using util::ZipfianGenerator;

std::vector<KeyDistribution> KeyDistribution::valueList;

KeyDistribution::StaticConstructor::StaticConstructor()
{
}

KeyDistribution::StaticConstructor KeyDistribution::staticConstructor;
int KeyDistribution::nextOrdinal = 0;
KeyDistribution::KeyDistribution(const std::wstring &name, InnerEnum innerEnum) : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
}

bool KeyDistribution::operator == (const KeyDistribution &other)
{
	return this->ordinalValue == other.ordinalValue;
}

bool KeyDistribution::operator != (const KeyDistribution &other)
{
	return this->ordinalValue != other.ordinalValue;
}

std::vector<KeyDistribution> KeyDistribution::values()
{
	return valueList;
}

int KeyDistribution::ordinal()
{
	return ordinalValue;
}

std::wstring KeyDistribution::toString()
{
	return nameValue;
}

KeyDistribution KeyDistribution::valueOf(const std::wstring &name)
{
	for (auto enumInstance : KeyDistribution::valueList)
	{
		if (enumInstance.nameValue == name)
		{
			return enumInstance;
		}
	}
}

KeyDistribution Parameters::distribution = KeyDistribution::Uniform;
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
std::wstring Parameters::benchClassName = std::wstring(L"skiplists.lockfree.NonBlockingFriendlySkipListMap");
bool Parameters::rangeQueries = false;
int Parameters::minRangeSize = 10;
int Parameters::maxRangeSize = 20;
bool Parameters::countKeysInRange = true;
double Parameters::rebalanceProbPerc = 2;
double Parameters::sortedRebalanceRatio = 1.8;
FillType Parameters::fillType = FillType::Random;
java::util::concurrent::atomic::AtomicInteger *Parameters::compactionsNum = new java::util::concurrent::atomic::AtomicInteger(0);
java::util::concurrent::atomic::AtomicInteger *Parameters::engagedChunks = new java::util::concurrent::atomic::AtomicInteger(0);
bool Parameters::countCompactions = false;
}