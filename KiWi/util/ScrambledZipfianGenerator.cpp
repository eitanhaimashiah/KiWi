#include "ScrambledZipfianGenerator.h"
#include "ZipfianGenerator.h"
#include "Utils.h"

namespace util
{
    ScrambledZipfianGenerator::ScrambledZipfianGenerator(long long _items) : ScrambledZipfianGenerator(0,_items - 1) {}

	ScrambledZipfianGenerator::ScrambledZipfianGenerator(long long _min, long long _max) :
    ScrambledZipfianGenerator(_min,_max,ZipfianGenerator::ZIPFIAN_CONSTANT) {}

    ScrambledZipfianGenerator::ScrambledZipfianGenerator(long long min, long long max, double _zipfianconstant) :
    _min(min), _max(max), _itemcount(max-min+1)
	{
        if (_zipfianconstant == USED_ZIPFIAN_CONSTANT)
        {
            gen = ZipfianGenerator(0,ITEM_COUNT,_zipfianconstant,ZETAN);
        }
        else
        {
            gen = ZipfianGenerator(0,ITEM_COUNT,_zipfianconstant);
        }
	}

	int ScrambledZipfianGenerator::nextInt()
	{
		return static_cast<int>(nextLong());
	}

    std::pair<int, int> ScrambledZipfianGenerator::nextInterval()
	{
		throw NotImplementedException();
	}

	long long ScrambledZipfianGenerator::nextLong()
	{
		long long ret = gen.nextLong();
		ret = _min + Utils::FNVhash64(ret) % _itemcount;
		setLastInt(static_cast<int>(ret));
		return ret;
	}

	double ScrambledZipfianGenerator::mean()
	{
		return (static_cast<double>((static_cast<long long>(_min)) + static_cast<long long>(_max))) / 2.0;
	}
}
