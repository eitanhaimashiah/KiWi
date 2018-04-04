#ifndef UniqueRandomIntGenerator_h
#define UniqueRandomIntGenerator_h

#include <vector>
#include <set>
#include "Utils.h"

using namespace std;

namespace util
{
	class UniqueRandomIntGenerator
	{
	private:
		vector<int> values;
		int idx = 0;

	public:
		UniqueRandomIntGenerator(int from, int to);

        UniqueRandomIntGenerator(set<int> &sourceValues);

		virtual bool hasNext();

		virtual int next();

		virtual void reset();

		virtual int last();

		virtual vector<int> &getValues();

	};

}

#endif /* UniqueRandomIntGenerator_h */
