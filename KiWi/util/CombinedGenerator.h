#ifndef CombinedGenerator_h
#define CombinedGenerator_h

#include <unordered_map>
#include <vector>
#include "UniqueRandomIntGenerator.h"
#include "Utils.h"

//JAVA TO C++ CONVERTER NOTE: Forward class declarations:
namespace util { class UniqueRandomIntGenerator; }

using namespace std;

namespace util
{
	class CombinedGenerator
	{
	private:
		vector<int> prefixes;
		unordered_map<int, UniqueRandomIntGenerator*> suffixMap;
		int idx = 0;
		static constexpr int shift = 2;

	public:
		CombinedGenerator(vector<int> &prefixValues);

		virtual bool hasNext();

		virtual int next();

		static int getShift();

		virtual void reset();

	private:
		int combineNumber(int prefix, int suffix);
	};
}

#endif /* CombinedGenerator_h */
