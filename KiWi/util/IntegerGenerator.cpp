

#include "IntegerGenerator.h"

namespace util
{

	void IntegerGenerator::setLastInt(int last)
	{
		lastint = last;
	}

	wstring IntegerGenerator::nextString()
	{
		return L"" + to_wstring(nextInt());
	}

	wstring IntegerGenerator::lastString()
	{
		return L"" + to_wstring(lastInt());
	}

	int IntegerGenerator::lastInt()
	{
		return lastint;
	}
}
