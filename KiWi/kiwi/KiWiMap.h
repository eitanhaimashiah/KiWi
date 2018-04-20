#ifndef KiWiMap_h
#define KiWiMap_h

#include <vector>
#include <map>
#include <set>
#include <utility>
#include <memory>

#include "KiWi.h"
#include "ChunkInt.h"
#include "CompositionalMap.h"

using namespace std;

namespace kiwi
{
	class KiWiMap : public CompositionalMap<int, int>
	{
	public:
        /***************	Members				***************/
        static bool SupportScan;
		static int RebalanceSize;

		KiWi<int, int> kiwi;

        /***************	Constructors		***************/
		KiWiMap();

        /***************	Methods				***************/
        /** same as put - always puts the new value! even if the key is not absent, it is updated */
		int putIfAbsent(int k, int v) override;

        /** requires full scan() for atomic size() */
		int size() override;

        /** not implemented ATM - can be implemented with chunk.findFirst() */
		bool isEmpty();

		int* get(int k);

		int put(int k, int v);

        /** same as put(key,null) - which signifies to KiWi that the item i removed */
		int remove(int k);

		int getRange(vector<int> &result, int min, int max) override;

        /** same as put(key,val) for each item */
		void putAll(map<int, int> map);

        /** Same as get(key) != null **/
		bool containsKey(int k);

        /** Clear is not really an option (can be implemented non-safe inside KiWi) - we just create new kiwi **/
		void clear() override;

        /** Not implemented - can scan all & return keys **/
		set<int> *keySet();

        /** Not implemented - can scan all & return values **/
		set<int> *values();

        /** Not implemented - can scan all & create entries **/
		set<pair<int, int>> entrySet();

        /** Not implemented - can scan all & search **/
		bool containsValue(int v);

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
