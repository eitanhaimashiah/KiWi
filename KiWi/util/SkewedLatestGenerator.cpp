

#include "SkewedLatestGenerator.h"
#include "CounterGenerator.h"
#include "ZipfianGenerator.h"
#include "Pair.h"

namespace util
{
	using sun::reflect::generics::reflectiveObjects::NotImplementedException;

	SkewedLatestGenerator::SkewedLatestGenerator(CounterGenerator *basis)
	{
		_basis = basis;
		_zipfian = new ZipfianGenerator(_basis->lastInt());
		nextInt();
	}

	int SkewedLatestGenerator::nextInt()
	{
		int max = _basis->lastInt();
		int nextint = max - _zipfian->nextInt(max);
		setLastInt(nextint);
		return nextint;
	}

	Pair<Integer, Integer> *SkewedLatestGenerator::nextInterval()
	{
		throw NotImplementedException();
	}

	void SkewedLatestGenerator::main(std::vector<std::wstring> &args)
	{
		CounterGenerator tempVar(1000);
		SkewedLatestGenerator *gen = new SkewedLatestGenerator(&tempVar);
		for (int i = 0; i < std::stoi(args[0]); i++)
		{
			std::wcout << gen->nextString() << std::endl;
		}

	}

	double SkewedLatestGenerator::mean()
	{
		throw UnsupportedOperationException(L"Can't compute mean of non-stationary distribution!");
	}
}
