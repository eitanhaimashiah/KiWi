#include "IntegerGenerator.h"
#include <string>
#include <vector>
#include <cmath>

//JAVA TO C++ CONVERTER NOTE: Forward class declarations:
namespace util { template<typename Ltypename R>class Pair; }

/// <summary>
/// Copyright (c) 2010 Yahoo! Inc. All rights reserved.                                                                                                                             
/// 
/// Licensed under the Apache License, Version 2.0 (the "License"); you                                                                                                             
/// may not use this file except in compliance with the License. You                                                                                                                
/// may obtain a copy of the License at                                                                                                                                             
/// 
/// http://www.apache.org/licenses/LICENSE-2.0                                                                                                                                      
/// 
/// Unless required by applicable law or agreed to in writing, software                                                                                                             
/// distributed under the License is distributed on an "AS IS" BASIS,                                                                                                               
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or                                                                                                                 
/// implied. See the License for the specific language governing                                                                                                                    
/// permissions and limitations under the License. See accompanying                                                                                                                 
/// LICENSE file.                                                                                                                                                                   
/// </summary>

namespace util
{


	/// <summary>
	/// A generator of a zipfian distribution. It produces a sequence of items, such that some items are more popular than others, according
	/// to a zipfian distribution. When you construct an instance of this class, you specify the number of items in the set to draw from, either
	/// by specifying an itemcount (so that the sequence is of items from 0 to itemcount-1) or by specifying a min and a max (so that the sequence is of 
	/// items from min to max inclusive). After you construct the instance, you can change the number of items by calling nextInt(itemcount) or nextLong(itemcount).
	/// 
	/// Note that the popular items will be clustered together, e.g. item 0 is the most popular, item 1 the second most popular, and so on (or min is the most 
	/// popular, min+1 the next most popular, etc.) If you don't want this clustering, and instead want the popular items scattered throughout the 
	/// item space, then use ScrambledZipfianGenerator instead.
	/// 
	/// Be aware: initializing this generator may take a long time if there are lots of items to choose from (e.g. over a minute
	/// for 100 million objects). This is because certain mathematical values need to be computed to properly generate a zipfian skew, and one of those
	/// values (zeta) is a sum sequence from 1 to n, where n is the itemcount. Note that if you increase the number of items in the set, we can compute
	/// a new zeta incrementally, so it should be fast unless you have added millions of items. However, if you decrease the number of items, we recompute
	/// zeta from scratch, so this can take a long time. 
	/// 
	/// The algorithm used here is from "Quickly Generating Billion-Record Synthetic Databases", Jim Gray et al, SIGMOD 1994.
	/// </summary>
	class ZipfianGenerator : public IntegerGenerator
	{
	public:
		static constexpr double ZIPFIAN_CONSTANT = 0.99;

		/// <summary>
		/// Number of items.
		/// </summary>
		long long items = 0;

		/// <summary>
		/// Min item to generate.
		/// </summary>
		long long base = 0;

		/// <summary>
		/// The zipfian constant to use.
		/// </summary>
		double zipfianconstant = 0;

		/// <summary>
		/// Computed parameters for generating the distribution.
		/// </summary>
		double alpha = 0, zetan = 0, eta = 0, theta = 0, zeta2theta = 0;

		/// <summary>
		/// The number of items used to compute zetan the last time.
		/// </summary>
		long long countforzeta = 0;

		/// <summary>
		/// Flag to prevent problems. If you increase the number of items the zipfian generator is allowed to choose from, this code will incrementally compute a new zeta
		/// value for the larger itemcount. However, if you decrease the number of items, the code computes zeta from scratch; this is expensive for large itemsets.
		/// Usually this is not intentional; e.g. one thread thinks the number of items is 1001 and calls "nextLong()" with that item count; then another thread who thinks the 
		/// number of items is 1000 calls nextLong() with itemcount=1000 triggering the expensive recomputation. (It is expensive for 100 million items, not really for 1000 items.) Why
		/// did the second thread think there were only 1000 items? maybe it read the item count before the first thread incremented it. So this flag allows you to say if you really do
		/// want that recomputation. If true, then the code will recompute zeta if the itemcount goes down. If false, the code will assume itemcount only goes up, and never recompute. 
		/// </summary>
		bool allowitemcountdecrease = false;

		/// <summary>
		///***************************** Constructors ************************************* </summary>

		/// <summary>
		/// Create a zipfian generator for the specified number of items. </summary>
		/// <param name="_items"> The number of items in the distribution. </param>
		ZipfianGenerator(long long _items);

		/// <summary>
		/// Create a zipfian generator for items between min and max. </summary>
		/// <param name="_min"> The smallest integer to generate in the sequence. </param>
		/// <param name="_max"> The largest integer to generate in the sequence. </param>
		ZipfianGenerator(long long _min, long long _max);

		/// <summary>
		/// Create a zipfian generator for the specified number of items using the specified zipfian constant.
		/// </summary>
		/// <param name="_items"> The number of items in the distribution. </param>
		/// <param name="_zipfianconstant"> The zipfian constant to use. </param>
		ZipfianGenerator(long long _items, double _zipfianconstant);

		/// <summary>
		/// Create a zipfian generator for items between min and max (inclusive) for the specified zipfian constant. </summary>
		/// <param name="min"> The smallest integer to generate in the sequence. </param>
		/// <param name="max"> The largest integer to generate in the sequence. </param>
		/// <param name="_zipfianconstant"> The zipfian constant to use. </param>
		ZipfianGenerator(long long min, long long max, double _zipfianconstant);

		/// <summary>
		/// Create a zipfian generator for items between min and max (inclusive) for the specified zipfian constant, using the precomputed value of zeta.
		/// </summary>
		/// <param name="min"> The smallest integer to generate in the sequence. </param>
		/// <param name="max"> The largest integer to generate in the sequence. </param>
		/// <param name="_zipfianconstant"> The zipfian constant to use. </param>
		/// <param name="_zetan"> The precomputed zeta constant. </param>
		ZipfianGenerator(long long min, long long max, double _zipfianconstant, double _zetan);

		/// <summary>
		///*********************************************************************** </summary>

		/// <summary>
		/// Compute the zeta constant needed for the distribution. Do this from scratch for a distribution with n items, using the 
		/// zipfian constant theta. Remember the value of n, so if we change the itemcount, we can recompute zeta.
		/// </summary>
		/// <param name="n"> The number of items to compute zeta over. </param>
		/// <param name="theta"> The zipfian constant. </param>
		virtual double zeta(long long n, double theta);

		/// <summary>
		/// Compute the zeta constant needed for the distribution. Do this from scratch for a distribution with n items, using the 
		/// zipfian constant theta. This is a static version of the function which will not remember n. </summary>
		/// <param name="n"> The number of items to compute zeta over. </param>
		/// <param name="theta"> The zipfian constant. </param>
		static double zetastatic(long long n, double theta);

		/// <summary>
		/// Compute the zeta constant needed for the distribution. Do this incrementally for a distribution that
		/// has n items now but used to have st items. Use the zipfian constant theta. Remember the new value of 
		/// n so that if we change the itemcount, we'll know to recompute zeta.
		/// </summary>
		/// <param name="st"> The number of items used to compute the last initialsum </param>
		/// <param name="n"> The number of items to compute zeta over. </param>
		/// <param name="theta"> The zipfian constant. </param>
		/// <param name="initialsum"> The value of zeta we are computing incrementally from. </param>
		virtual double zeta(long long st, long long n, double theta, double initialsum);

		/// <summary>
		/// Compute the zeta constant needed for the distribution. Do this incrementally for a distribution that
		/// has n items now but used to have st items. Use the zipfian constant theta. Remember the new value of 
		/// n so that if we change the itemcount, we'll know to recompute zeta. </summary>
		/// <param name="st"> The number of items used to compute the last initialsum </param>
		/// <param name="n"> The number of items to compute zeta over. </param>
		/// <param name="theta"> The zipfian constant. </param>
		/// <param name="initialsum"> The value of zeta we are computing incrementally from. </param>
		static double zetastatic(long long st, long long n, double theta, double initialsum);

		/// <summary>
		///************************************************************************************* </summary>

		/// <summary>
		/// Generate the next item. this distribution will be skewed toward lower integers; e.g. 0 will
		/// be the most popular, 1 the next most popular, etc. </summary>
		/// <param name="itemcount"> The number of items in the distribution. </param>
		/// <returns> The next item in the sequence. </returns>
		virtual int nextInt(int itemcount);

		/// <summary>
		/// Generate the next item as a long.
		/// </summary>
		/// <param name="itemcount"> The number of items in the distribution. </param>
		/// <returns> The next item in the sequence. </returns>
		virtual long long nextLong(long long itemcount);

		/// <summary>
		/// Return the next value, skewed by the Zipfian distribution. The 0th item will be the most popular, followed by the 1st, followed
		/// by the 2nd, etc. (Or, if min != 0, the min-th item is the most popular, the min+1th item the next most popular, etc.) If you want the
		/// popular items scattered throughout the item space, use ScrambledZipfianGenerator instead.
		/// </summary>
		int nextInt() override;

		Pair<Integer, Integer> *nextInterval() override;

		/// <summary>
		/// Return the next value, skewed by the Zipfian distribution. The 0th item will be the most popular, followed by the 1st, followed
		/// by the 2nd, etc. (Or, if min != 0, the min-th item is the most popular, the min+1th item the next most popular, etc.) If you want the
		/// popular items scattered throughout the item space, use ScrambledZipfianGenerator instead.
		/// </summary>
		virtual long long nextLong();

		static void main(std::vector<std::wstring> &args);

		/// <summary>
		/// @todo Implement ZipfianGenerator.mean()
		/// </summary>
		double mean() override;
	};

}
