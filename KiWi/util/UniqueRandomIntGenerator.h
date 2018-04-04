#ifndef UniqueRandomIntGenerator_h
#define UniqueRandomIntGenerator_h

#include <vector>
#include <set>
#include "Utils.h"

namespace util
{
	class UniqueRandomIntGenerator
	{
	private:
		std::vector<int> values;
		int idx = 0;

	public:
		UniqueRandomIntGenerator(int from, int to);

        UniqueRandomIntGenerator(std::set<int> &sourceValues);

		virtual bool hasNext();

		virtual int next();

		virtual void reset();

		virtual int last();

		virtual std::vector<int> &getValues();

	};

}

#endif /* UniqueRandomIntGenerator_h */
