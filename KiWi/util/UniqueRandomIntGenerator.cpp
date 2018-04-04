#include "UniqueRandomIntGenerator.h"

using namespace std;

namespace util
{
	UniqueRandomIntGenerator::UniqueRandomIntGenerator(int from, int to)
	{
		values = vector<int>(to - from + 1);
		for (int i = from; i < to; ++i)
		{
			values.push_back(i);
		}

		idx = static_cast<int>(values.size() - 1);
	}

	UniqueRandomIntGenerator::UniqueRandomIntGenerator(set<int> &sourceValues)
	{
		values = vector<int>(sourceValues.size());

		for (auto val : sourceValues)
		{
			values.push_back(val);
		}

        idx = static_cast<int>(values.size() - 1);
    }

	bool UniqueRandomIntGenerator::hasNext()
	{
		return idx >= 0;
	}

	int UniqueRandomIntGenerator::next()
	{
		if (idx < 0)
		{
			return -1;
		}
        int randIdx = Utils::nextInt(idx + 1);

		int nextVal = values[randIdx];

		//swap values here
		values[randIdx] = values[idx];
		values[idx] = nextVal;


		idx--;

		return nextVal;
	}

	void UniqueRandomIntGenerator::reset()
	{
        idx = static_cast<int>(values.size() - 1);
	}

	int UniqueRandomIntGenerator::last()
	{
		int last = idx + 1;
		if (last >= values.size())
		{
			return -1;
		}
		return values[last];
	}

	vector<int> &UniqueRandomIntGenerator::getValues()
	{
		return values;
	}
}
