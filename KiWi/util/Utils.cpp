#include "Utils.h"

namespace util
{
    int Utils::nextInt(int upper)
    {
        static thread_local default_random_engine gen;
        uniform_int_distribution<int> dis (0, upper-1);
        return dis(gen);
    }

    double Utils::nextDouble()
    {
        static thread_local default_random_engine gen;
        uniform_real_distribution<double> dis;
        return dis(gen);
    }
    
	long long Utils::hash(long long val)
	{
        return FNVhash64(val);
	}

	int Utils::FNVhash32(int val)
	{
        //from http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash
        int hashval = FNV_offset_basis_32;

        for (int i = 0; i < 4; i++)
        {
            int octet = val&0x00ff;
            val = val >> 8;

            hashval = hashval ^ octet;
            hashval = hashval * FNV_prime_32;
            //hashval = hashval ^ octet;
        }
        return abs(hashval);
	}

	long long Utils::FNVhash64(long long val)
	{
        //from http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash
        long long hashval = FNV_offset_basis_64;

        for (int i = 0; i < 8; i++)
        {
            long long octet = val&0x00ff;
            val = val >> 8;

            hashval = hashval ^ octet;
            hashval = hashval * FNV_prime_64;
            //hashval = hashval ^ octet;
        }
        return abs(hashval);
	}
}
