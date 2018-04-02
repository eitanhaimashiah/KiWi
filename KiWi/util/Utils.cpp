

#include "Utils.h"

namespace util
{

java::util::Random *const Utils::rand = new java::util::Random();
ThreadLocal<java::util::Random*> *const Utils::rng = new ThreadLocal<java::util::Random*>();

	Random *Utils::random()
	{
	  Random *ret = rng->get();
	  if (ret == nullptr)
	  {
		ret = new Random(rand->nextLong());
		rng->set(ret);
	  }
	  return ret;
	}

	std::wstring Utils::ASCIIString(int length)
	{
			 int interval = L'~' - L' ' + 1;

	  std::vector<char> buf(length);
	  random()->nextBytes(buf);
	  for (int i = 0; i < length; i++)
	  {
		if (buf[i] < 0)
		{
		  buf[i] = static_cast<char>((-buf[i] % interval) + L' ');
		}
		else
		{
		  buf[i] = static_cast<char>((buf[i] % interval) + L' ');
		}
	  }
	  return std::wstring(buf);
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
			 return std::abs(hashval);
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
			 return std::abs(hashval);
	}
}
