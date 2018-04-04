#include "SkewedLatestGenerator.h"

using namespace std;

namespace util
{
    SkewedLatestGenerator::SkewedLatestGenerator(CounterGenerator basis) : _basis(basis),_zipfian(_basis.lastInt())
	{
		nextInt();
	}

	int SkewedLatestGenerator::nextInt()
	{
		int max = _basis.lastInt();
		int nextint = max - _zipfian.nextInt(max);
		setLastInt(nextint);
		return nextint;
	}

    pair<int, int> SkewedLatestGenerator::nextInterval()
	{
		throw NotImplementedException();
	}

	double SkewedLatestGenerator::mean()
	{
		throw UnsupportedOperationException("Can't compute mean of non-stationary distribution!");
	}
}
