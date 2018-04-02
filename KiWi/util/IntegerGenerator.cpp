

#include "IntegerGenerator.h"
#include "Pair.h"

namespace util
{

	void IntegerGenerator::setLastInt(int last)
	{
		lastint = last;
	}

	std::wstring IntegerGenerator::nextString()
	{
		return L"" + std::to_wstring(nextInt());
	}

	std::wstring IntegerGenerator::lastString()
	{
		return L"" + std::to_wstring(lastInt());
	}

	int IntegerGenerator::lastInt()
	{
		return lastint;
	}
}
