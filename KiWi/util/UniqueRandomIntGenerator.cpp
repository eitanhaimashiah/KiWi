

#include "UniqueRandomIntGenerator.h"

namespace util
{

	UniqueRandomIntGenerator::UniqueRandomIntGenerator(int from, int to)
	{
		values = std::vector<Integer>(to - from + 1);
		for (int i = from; i < to; ++i)
		{
			values.push_back(i);
		}

		idx = values.size() - 1;
	}

	UniqueRandomIntGenerator::UniqueRandomIntGenerator(Set<Integer> *sourceValues)
	{
		assert(sourceValues != nullptr);

		values = std::vector<Integer>(sourceValues->size());

		for (auto val : sourceValues)
		{
			values.push_back(val);
		}

		idx = values.size() - 1;
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
		int randIdx = rand->nextInt(idx + 1);

		Integer nextVal = values[randIdx];

		//swap values here
		values[randIdx] = values[idx];
		values[idx] = nextVal;


		idx--;

		return nextVal;
	}

	void UniqueRandomIntGenerator::reset()
	{
		idx = values.size() - 1;
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

	std::vector<Integer> UniqueRandomIntGenerator::getValues()
	{
		return values;
	}
}
