#include "CompositionalMap.h"
#include <unordered_map>
#include <vector>

namespace kiwi
{



	/// <summary>
	/// Created by msulamy on 7/27/15.
	/// </summary>
	class KiWiMap : public CompositionalMap<Integer, Integer>
	{
		/// <summary>
		///*************	Constants			************** </summary>

		/// <summary>
		///*************	Members				************** </summary>
	public:
		static bool SupportScan;
		static int RebalanceSize;

		KiWi<Integer, Integer> *kiwi;

		/// <summary>
		///*************	Constructors		************** </summary>
		virtual ~KiWiMap()
		{
			delete kiwi;
		}

		KiWiMap();

		/// <summary>
		///*************	Methods				************** </summary>

		/// <summary>
		/// same as put - always puts the new value! even if the key is not absent, it is updated </summary>
		Integer putIfAbsent(Integer k, Integer v) override;

		/// <summary>
		/// requires full scan() for atomic size() </summary>
		int size() override;

		/// <summary>
		/// not implemented ATM - can be implemented with chunk.findFirst() </summary>
		bool isEmpty() override;

		Integer get(void *o) override;

		Integer put(Integer k, Integer v) override;

		/// <summary>
		/// same as put(key,null) - which signifies to KiWi that the item is removed </summary>
		Integer remove(void *o) override;

		int getRange(std::vector<Integer> &result, Integer min, Integer max) override;

		/// <summary>
		/// same as put(key,val) for each item </summary>
		template<typename T1, typename T1>
//JAVA TO C++ CONVERTER TODO TASK: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public void putAll(java.util.Map<? extends Integer, ? extends Integer> map)
		void putAll(std::unordered_map<T1> map);

		/// <summary>
		/// Same as get(key) != null * </summary>
		bool containsKey(void *o) override;

		/// <summary>
		/// Clear is not really an option (can be implemented non-safe inside KiWi) - we just create new kiwi * </summary>
		void clear() override;

		/// <summary>
		/// Not implemented - can scan all & return keys * </summary>
		Set<Integer> *keySet() override;

		/// <summary>
		/// Not implemented - can scan all & return values * </summary>
		Collection<Integer> *values() override;

		/// <summary>
		/// Not implemented - can scan all & create entries * </summary>
		Set<Entry<Integer, Integer>*> *entrySet() override;

		/// <summary>
		/// Not implemented - can scan all & search * </summary>
		bool containsValue(void *o) override;

		virtual void compactAllSerial();
		virtual int debugCountDups();
		virtual int debugCountKeys();
		virtual void debugPrint();

		virtual int debugCountDuplicates();
		virtual int debugCountChunks();

		virtual void calcChunkStatistics();
	};

}
