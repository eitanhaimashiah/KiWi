#include <unordered_map>
#include <vector>
#include "exceptionhelper.h"

//JAVA TO C++ CONVERTER NOTE: Forward class declarations:
namespace kiwi { template<typename Ktypename V>class KiWi; }

namespace kiwi
{



	/// <summary>
	/// Created by msulamy on 7/27/15.
	/// </summary>
	class KiWiMapVLK : public CompositionalMap<Integer, Integer>
	{
		/// <summary>
		///*************	Constants			************** </summary>

		/// <summary>
		///*************	Members				************** </summary>
	private:
		KiWi<Cell*, Cell*> *kiwi;

		/// <summary>
		///*************	Constructors		************** </summary>
	public:
		virtual ~KiWiMapVLK()
		{
			delete kiwi;
		}

		KiWiMapVLK();

		/// <summary>
		///*************	Methods				************** </summary>

		Integer putIfAbsent(Integer k, Integer v) override;

		int size() override;
		bool isEmpty() override;

		Integer get(void *o) override;

		Integer put(Integer k, Integer v) override;

		Integer remove(void *o) override;

		template<typename T1, typename T1>
//JAVA TO C++ CONVERTER TODO TASK: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public void putAll(java.util.Map<? extends Integer, ? extends Integer> map)
		void putAll(std::unordered_map<T1> map);

		int getRange(std::vector<Integer> &result, Integer min, Integer max) override;

		/// <summary>
		/// Same as get(key) != null * </summary>
		bool containsKey(void *o) override;

		/// <summary>
		/// Not supported - can be implemented in a non-safe manner * </summary>
		void clear() override;

		/// <summary>
		/// Scan all & return keys * </summary>
		Set<Integer> *keySet() override;

		/// <summary>
		/// Scan all & return values * </summary>
		Collection<Integer> *values() override;

		/// <summary>
		/// Scan all & create entries * </summary>
		Set<Entry<Integer, Integer>*> *entrySet() override;

		/// <summary>
		/// Scan all & search * </summary>
		bool containsValue(void *o) override;

	private:
		Cell *cellFromInt(Integer n);
		Integer cellToInt(Cell *c);

	public:
		virtual void debugPrint();
	};

}