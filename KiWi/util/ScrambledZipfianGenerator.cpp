

#include "ScrambledZipfianGenerator.h"
#include "ZipfianGenerator.h"
#include "Pair.h"
#include "Utils.h"

namespace util
{
	using sun::reflect::generics::reflectiveObjects::NotImplementedException;

	ScrambledZipfianGenerator::ScrambledZipfianGenerator(long long _items) : ScrambledZipfianGenerator(0,_items - 1)
	{
	}

	ScrambledZipfianGenerator::ScrambledZipfianGenerator(long long _min, long long _max) : ScrambledZipfianGenerator(_min,_max,ZipfianGenerator::ZIPFIAN_CONSTANT)
	{
	}

	ScrambledZipfianGenerator::ScrambledZipfianGenerator(long long min, long long max, double _zipfianconstant)
	{
	_min = min;
	_max = max;
	_itemcount = _max - _min + 1;
	if (_zipfianconstant == USED_ZIPFIAN_CONSTANT)
	{
		gen = new ZipfianGenerator(0,ITEM_COUNT,_zipfianconstant,ZETAN);
	}
	else
	{
		gen = new ZipfianGenerator(0,ITEM_COUNT,_zipfianconstant);
	}
	}

	int ScrambledZipfianGenerator::nextInt()
	{
		return static_cast<int>(nextLong());
	}

	Pair<Integer, Integer> *ScrambledZipfianGenerator::nextInterval()
	{
		throw NotImplementedException();
	}

	long long ScrambledZipfianGenerator::nextLong()
	{
		long long ret = gen->nextLong();
		ret = _min + Utils::FNVhash64(ret) % _itemcount;
		setLastInt(static_cast<int>(ret));
		return ret;
	}

	void ScrambledZipfianGenerator::main(std::vector<std::wstring> &args)
	{
		double newzetan = ZipfianGenerator::zetastatic(ITEM_COUNT,ZipfianGenerator::ZIPFIAN_CONSTANT);
		std::wcout << L"zetan: " << newzetan << std::endl;
		exit(0);

		ScrambledZipfianGenerator *gen = new ScrambledZipfianGenerator(10000);

		for (int i = 0; i < 1000000; i++)
		{
			std::wcout << L"" << gen->nextInt() << std::endl;
		}
	}

	double ScrambledZipfianGenerator::mean()
	{
		return (static_cast<double>((static_cast<long long>(_min)) + static_cast<long long>(_max))) / 2.0;
	}
}
