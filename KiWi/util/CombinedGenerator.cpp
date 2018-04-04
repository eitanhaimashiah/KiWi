#include "CombinedGenerator.h"

namespace util
{

	CombinedGenerator::CombinedGenerator(vector<int> &prefixValues)
	{
		assert(prefixValues.size() > 0);

		prefixes = vector<Integer>(prefixValues.size());
		suffixMap = unordered_map<>(prefixValues.size());

		for (auto val : prefixValues)
		{
			prefixes.push_back(val);
			UniqueRandomIntGenerator tempVar(0, 2);
			suffixMap.emplace(val, &tempVar);
		}

		idx = prefixes.size() - 1;
	}

	bool CombinedGenerator::hasNext()
	{
		return idx >= 0;
	}

	int CombinedGenerator::next()
	{
        int randIdx = Utils::nextInt(idx + 1);
		Integer prefix = prefixes[randIdx];

		UniqueRandomIntGenerator *gen = suffixMap[prefix];
		int suffix = gen->next();

		if (!gen->hasNext())
		{
			prefixes[randIdx] = prefixes[idx];
			prefixes[idx] = prefix;
			idx--;
		}

		return combineNumber(prefix, suffix);
	}

	int CombinedGenerator::getShift()
	{
		return shift;
	}

	void CombinedGenerator::reset()
	{
		for (auto gen : suffixMap)
		{
			gen->second.reset();
		}

		idx = prefixes.size() - 1;
	}

	int CombinedGenerator::combineNumber(int prefix, int suffix)
	{
		int num = prefix << shift;
		num = num | suffix;

		return num;
	}
}
