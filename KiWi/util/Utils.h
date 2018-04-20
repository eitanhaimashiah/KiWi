#ifndef Utils_h
#define Utils_h

#include <random>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

namespace util
{
    /**
     * Utility functions.
     */
	class Utils
	{
    public:
        /**
         * Generate a uniformly distributed random integer value between 0 (inclusive) and the specified value (exclusive)
         */
        static int nextInt(int upper);

        /**
         * Generate a uniformly distributed random double value between 0.0 and 1.0.
         */
        static double nextDouble();
        
        /**
         * Hash an integer value.
         */
        static long long hash(long long val);

        static constexpr int FNV_offset_basis_32 = 0x811c9dc5;
        static constexpr int FNV_prime_32 = 16777619;

        /**
         * 32 bit FNV hash. Produces more "random" hashes than (say) String.hashCode().
         *
         * @param val The value to hash.
         * @return The hash value
         */
        static int FNVhash32(int val);

        static constexpr long long FNV_offset_basis_64 = 0xCBF29CE484222325LL;
        static constexpr long long FNV_prime_64 = 1099511628211LL;

        /**
         * 64 bit FNV hash. Produces more "random" hashes than (say) String.hashCode().
         *
         * @param val The value to hash.
         * @return The hash value
         */
        static long long FNVhash64(long long val);
    };

}

#endif /* Utils_h */
