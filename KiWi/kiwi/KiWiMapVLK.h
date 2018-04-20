#ifndef KiWiMapVLK_h
#define KiWiMapVLK_h

#include <set>
#include <map>
#include <vector>
#include <utility>
#include <memory>

#include "ChunkCell.h"
#include "CompositionalMap.h"
#include "exceptionhelper.h"
#include "KiWi.h"

using namespace std;

namespace kiwi
{
	class KiWiMapVLK : public CompositionalMap<int, int>
	{
    /***************	Members				***************/
	private:
		KiWi<Cell, Cell> kiwi;

    /***************	Constructors		***************/
	public:
		KiWiMapVLK();

    /***************	Methods				***************/

		int putIfAbsent(int k, int v) override;

		int size() override;
        bool isEmpty();

        int get(int k);

		int put(int k, int v);

		int remove(int k);

		void putAll(map<int, int> map);

		int getRange(vector<int> &result, int min, int max) override;

        /** Same as get(key) != null **/
		bool containsKey(int k);

        /** Not supported - can be implemented in a non-safe manner **/
		void clear() override;

        /** Scan all & return keys **/
		set<int> *keySet();

        /** Scan all & return values **/
		set<int> *values();

        /** Scan all & create entries **/
		set<pair<int, int>> entrySet();

        /** Scan all & search **/
		bool containsValue(int v);

	private:
		Cell cellFromInt(int n);
		int cellToInt(Cell *c);

	public:
		virtual void debugPrint();
	};

}

#endif /* KiWiMapVLK_h */
