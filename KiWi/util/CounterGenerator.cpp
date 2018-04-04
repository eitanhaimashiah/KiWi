#include <vector>
#include "CounterGenerator.h"

namespace util
{
	CounterGenerator::CounterGenerator(int countstart) : counter(countstart)
	{
		setLastInt(counter.load() - 1);
	}
    
    CounterGenerator::CounterGenerator(CounterGenerator const &other) : counter(other.counter.load())
    {
        setLastInt(counter.load() - 1);
    }
    
	int CounterGenerator::nextInt()
	{
        int ret = counter++;
		setLastInt(ret);
		return ret;
	}

    std::pair<int, int> CounterGenerator::nextInterval()
	{
		throw NotImplementedException();
	}

	int CounterGenerator::lastInt()
	{
        return counter.load() - 1;
	}

	double CounterGenerator::mean()
	{
		throw UnsupportedOperationException("Can't compute mean of non-stationary distribution!");
	}
}
