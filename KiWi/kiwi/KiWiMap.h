#ifndef KiWiMap_h
#define KiWiMap_h

#include "CompositionalMap.h"
#include <unordered_map>
#include <vector>

using namespace std;

namespace kiwi
{
	class KiWiMap : public CompositionalMap<Integer, Integer>
	{
        /***************	Members				***************/
	public:
		static bool SupportScan;
		static int RebalanceSize;

		KiWi<Integer, Integer> *kiwi;

        /***************	Constructors		***************/
		virtual ~KiWiMap()
		{
			delete kiwi;
		}

		KiWiMap();

        /***************	Methods				***************/

        /** same as put - always puts the new value! even if the key is not absent, it is updated */
		Integer putIfAbsent(Integer k, Integer v) override;

        /** requires full scan() for atomic size() */
		int size() override;

        /** not implemented ATM - can be implemented with chunk.findFirst() */
		bool isEmpty() override;

		Integer get(void *o) override;

		Integer put(Integer k, Integer v) override;

        /** same as put(key,null) - which signifies to KiWi that the item is removed */
		Integer remove(void *o) override;

		int getRange(vector<Integer> &result, Integer min, Integer max) override;

        /** same as put(key,val) for each item */
		template<typename T1, typename T1>
//JAVA TO C++ CONVERTER TODO TASK: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public void putAll(java.util.Map<? extends Integer, ? extends Integer> map)
		void putAll(unordered_map<T1> map);

        /** Same as get(key) != null **/
		bool containsKey(void *o) override;

        /** Clear is not really an option (can be implemented non-safe inside KiWi) - we just create new kiwi **/
		void clear() override;

        /** Not implemented - can scan all & return keys **/
		Set<Integer> *keySet() override;

        /** Not implemented - can scan all & return values **/
		Collection<Integer> *values() override;

        /** Not implemented - can scan all & create entries **/
		Set<Entry<Integer, Integer>*> *entrySet() override;

        /** Not implemented - can scan all & search **/
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

#endif /* KiWiMap_h */
