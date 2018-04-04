#ifndef ScrambledZipfianGenerator_h
#define ScrambledZipfianGenerator_h

#include <string>
#include <vector>
#include <iostream>
#include "ZipfianGenerator.h"

namespace util
{
    /**
     * A generator of a zipfian distribution. It produces a sequence of items, such that some items are more popular than others, according
     * to a zipfian distribution. When you construct an instance of this class, you specify the number of items in the set to draw from, either
     * by specifying an itemcount (so that the sequence is of items from 0 to itemcount-1) or by specifying a min and a max (so that the sequence is of
     * items from min to max inclusive). After you construct the instance, you can change the number of items by calling nextInt(itemcount) or nextLong(itemcount).
     *
     * Unlike @ZipfianGenerator, this class scatters the "popular" items across the itemspace. Use this, instead of @ZipfianGenerator, if you
     * don't want the head of the distribution (the popular items) clustered together.
     */
	class ScrambledZipfianGenerator : public IntegerGenerator
	{
	public:
		static constexpr double ZETAN = 26.46902820178302;
        static constexpr double USED_ZIPFIAN_CONSTANT = 0.99;
		static constexpr long long ITEM_COUNT = 10000000000LL;

		ZipfianGenerator gen;
		long long _min = 0, _max = 0, _itemcount = 0;

		/***************************** Constructors *************************************/

        /**
         * Create a zipfian generator for the specified number of items.
         * @param _items The number of items in the distribution.
         */
		ScrambledZipfianGenerator(long long _items);

        /**
         * Create a zipfian generator for items between min and max.
         * @param _min The smallest integer to generate in the sequence.
         * @param _max The largest integer to generate in the sequence.
         */
		ScrambledZipfianGenerator(long long _min, long long _max);

        /**
         * Create a zipfian generator for items between min and max (inclusive) for the specified zipfian constant. If you
         * use a zipfian constant other than 0.99, this will take a long time to complete because we need to recompute zeta.
         * @param min The smallest integer to generate in the sequence.
         * @param max The largest integer to generate in the sequence.
         * @param _zipfianconstant The zipfian constant to use.
         */
        ScrambledZipfianGenerator(long long min, long long max, double _zipfianconstant);

        /**************************************************************************************************/

        /**
         * Return the next int in the sequence.
         */
		int nextInt() override;

        std::pair<int, int> nextInterval() override;

        /**
         * Return the next long in the sequence.
         */
		virtual long long nextLong();

        /**
         * since the values are scrambled (hopefully uniformly), the mean is simply the middle of the range.
         */
		double mean() override;
	};

}

#endif /* ScrambledZipfianGenerator_h */
