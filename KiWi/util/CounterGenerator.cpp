

#include "CounterGenerator.h"
#include "Pair.h"

namespace util
{
	using sun::reflect::generics::reflectiveObjects::NotImplementedException;

	CounterGenerator::CounterGenerator(int countstart) : counter(new AtomicInteger(countstart))
	{
		setLastInt(counter->get() - 1);
	}

	int CounterGenerator::nextInt()
	{
		int ret = counter->getAndIncrement();
		setLastInt(ret);
		return ret;
	}

	Pair<Integer, Integer> *CounterGenerator::nextInterval()
	{
		throw NotImplementedException();
	}

	int CounterGenerator::lastInt()
	{
					return counter->get() - 1;
	}

	double CounterGenerator::mean()
	{
		throw UnsupportedOperationException(L"Can't compute mean of non-stationary distribution!");
	}
}
