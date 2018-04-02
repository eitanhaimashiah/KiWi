#include "IntegerGenerator.h"
#include <string>
#include <vector>
#include <iostream>

//JAVA TO C++ CONVERTER NOTE: Forward class declarations:
namespace util { class ZipfianGenerator; }
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
	/// Unlike @ZipfianGenerator, this class scatters the "popular" items across the itemspace. Use this, instead of @ZipfianGenerator, if you
	/// don't want the head of the distribution (the popular items) clustered together.
	/// </summary>
	class ScrambledZipfianGenerator : public IntegerGenerator
	{
	public:
		static constexpr double ZETAN = 26.46902820178302;
			static constexpr double USED_ZIPFIAN_CONSTANT = 0.99;
		static constexpr long long ITEM_COUNT = 10000000000LL;

		ZipfianGenerator *gen;
		long long _min = 0, _max = 0, _itemcount = 0;

		/// <summary>
		///***************************** Constructors ************************************* </summary>

		/// <summary>
		/// Create a zipfian generator for the specified number of items. </summary>
		/// <param name="_items"> The number of items in the distribution. </param>
		virtual ~ScrambledZipfianGenerator()
		{
			delete gen;
		}

		ScrambledZipfianGenerator(long long _items);

		/// <summary>
		/// Create a zipfian generator for items between min and max. </summary>
		/// <param name="_min"> The smallest integer to generate in the sequence. </param>
		/// <param name="_max"> The largest integer to generate in the sequence. </param>
		ScrambledZipfianGenerator(long long _min, long long _max);

		/// <summary>
		/// Create a zipfian generator for the specified number of items using the specified zipfian constant.
		/// </summary>
		/// <param name="_items"> The number of items in the distribution. </param>
		/// <param name="_zipfianconstant"> The zipfian constant to use. </param>
		/*
	// not supported, as the value of zeta depends on the zipfian constant, and we have only precomputed zeta for one zipfian constant
		public ScrambledZipfianGenerator(long _items, double _zipfianconstant)
		{
			this(0,_items-1,_zipfianconstant);
		}
	*/

		/// <summary>
		/// Create a zipfian generator for items between min and max (inclusive) for the specified zipfian constant. If you 
		/// use a zipfian constant other than 0.99, this will take a long time to complete because we need to recompute zeta. </summary>
		/// <param name="min"> The smallest integer to generate in the sequence. </param>
		/// <param name="max"> The largest integer to generate in the sequence. </param>
		/// <param name="_zipfianconstant"> The zipfian constant to use. </param>
			ScrambledZipfianGenerator(long long min, long long max, double _zipfianconstant);

		/// <summary>
		///*********************************************************************************************** </summary>

		/// <summary>
		/// Return the next int in the sequence.
		/// </summary>
		int nextInt() override;

		Pair<Integer, Integer> *nextInterval() override;

		/// <summary>
		/// Return the next long in the sequence.
		/// </summary>
		virtual long long nextLong();

		static void main(std::vector<std::wstring> &args);

		/// <summary>
		/// since the values are scrambled (hopefully uniformly), the mean is simply the middle of the range.
		/// </summary>
		double mean() override;
	};

}
