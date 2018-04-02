#include <unordered_map>
#include <vector>

//JAVA TO C++ CONVERTER NOTE: Forward class declarations:
namespace util { class UniqueRandomIntGenerator; }

namespace util
{


	/// <summary>
	/// Created by dbasin on 7/7/16.
	/// </summary>

	class CombinedGenerator
	{
	private:
		std::vector<Integer> prefixes;
		std::unordered_map<Integer, UniqueRandomIntGenerator*> suffixMap;
		int idx = 0;
		Random *random;
		static constexpr int shift = 2;

	public:
		virtual ~CombinedGenerator()
		{
			delete random;
		}

		CombinedGenerator(std::vector<Integer> &prefixValues);

		virtual bool hasNext();

		virtual int next();

		static int getShift();

		virtual void reset();

	private:
		int combineNumber(int prefix, int suffix);
	};


}
