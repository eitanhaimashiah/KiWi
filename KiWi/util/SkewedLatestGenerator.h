#ifndef SkewedLatestGenerator_h
#define SkewedLatestGenerator_h

#include <string>
#include <vector>
#include <iostream>
#include "IntegerGenerator.h"
#include "CounterGenerator.h"
#include "ZipfianGenerator.h"

namespace util
{
    /**
     * Generate a popularity distribution of items, skewed to favor recent items significantly more than older items.
     */
	class SkewedLatestGenerator : public IntegerGenerator
	{
	public:
		CounterGenerator _basis;
		ZipfianGenerator _zipfian;

		SkewedLatestGenerator(CounterGenerator basis);

        /**
         * Generate the next string in the distribution, skewed Zipfian favoring the items most recently returned by the basis generator.
         */
		int nextInt() override;

        std::pair<int, int> nextInterval() override;
                
		double mean() override;
	};
}

#endif /* SkewedLatestGenerator_h */
