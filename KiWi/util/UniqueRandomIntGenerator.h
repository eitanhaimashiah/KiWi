#include <vector>

namespace util
{


	/// <summary>
	/// Created by dbasin on 7/7/16.
	/// </summary>
	class UniqueRandomIntGenerator
	{
	private:
		std::vector<Integer> values;
		int idx = 0;
		Random *rand = new Random();

	public:
		virtual ~UniqueRandomIntGenerator()
		{
			delete rand;
		}

		UniqueRandomIntGenerator(int from, int to);

		UniqueRandomIntGenerator(Set<Integer> *sourceValues);

		virtual bool hasNext();

		virtual int next();

		virtual void reset();

		virtual int last();

		virtual std::vector<Integer> getValues();

	};

}
