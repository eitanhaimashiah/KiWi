#include <string>
#include <vector>
#include <cmath>

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
	/// Utility functions.
	/// </summary>
	class Utils
	{
  private:
	  static Random *const rand;
	  static ThreadLocal<Random*> *const rng;

  public:
	  static Random *random();
		  /// <summary>
		  /// Generate a random ASCII string of a given length.
		  /// </summary>
		  static std::wstring ASCIIString(int length);

		  /// <summary>
		  /// Hash an integer value.
		  /// </summary>
		  static long long hash(long long val);

		  static constexpr int FNV_offset_basis_32 = 0x811c9dc5;
		  static constexpr int FNV_prime_32 = 16777619;

		  /// <summary>
		  /// 32 bit FNV hash. Produces more "random" hashes than (say) String.hashCode().
		  /// </summary>
		  /// <param name="val"> The value to hash. </param>
		  /// <returns> The hash value </returns>
		  static int FNVhash32(int val);

		  static constexpr long long FNV_offset_basis_64 = 0xCBF29CE484222325LL;
		  static constexpr long long FNV_prime_64 = 1099511628211LL;

		  /// <summary>
		  /// 64 bit FNV hash. Produces more "random" hashes than (say) String.hashCode().
		  /// </summary>
		  /// <param name="val"> The value to hash. </param>
		  /// <returns> The hash value </returns>
		  static long long FNVhash64(long long val);
	};

}
