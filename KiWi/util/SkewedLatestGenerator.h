#include "IntegerGenerator.h"
#include <string>
#include <vector>
#include <iostream>

//JAVA TO C++ CONVERTER NOTE: Forward class declarations:
namespace util { class CounterGenerator; }
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
	/// Generate a popularity distribution of items, skewed to favor recent items significantly more than older items.
	/// </summary>
	class SkewedLatestGenerator : public IntegerGenerator
	{
	public:
		CounterGenerator *_basis;
		ZipfianGenerator *_zipfian;

		virtual ~SkewedLatestGenerator()
		{
			delete _basis;
			delete _zipfian;
		}

		SkewedLatestGenerator(CounterGenerator *basis);

		/// <summary>
		/// Generate the next string in the distribution, skewed Zipfian favoring the items most recently returned by the basis generator.
		/// </summary>
		int nextInt() override;

		Pair<Integer, Integer> *nextInterval() override;

		static void main(std::vector<std::wstring> &args);

		double mean() override;

	};

}
