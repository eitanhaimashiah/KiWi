#ifndef CounterGenerator_h
#define CounterGenerator_h

#include <atomic>
#include "IntegerGenerator.h"
#include "exceptionhelper.h"

using namespace std;

namespace util
{
	// Generates a sequence of integers 0, 1, ...
	class CounterGenerator : public IntegerGenerator
	{
	public:
        atomic<int> counter;

		CounterGenerator(int countstart);
        
        CounterGenerator(CounterGenerator const &other);

		/**
		 * If the generator returns numeric (integer) values, return the next value as an int. Default is to return -1, which
		 * is appropriate for generators that do not return numeric values.
		 */
		int nextInt() override;
        
        pair<int, int> nextInterval() override;

		int lastInt() override;
		double mean() override;
	};

}

#endif /* CounterGenerator_h */
