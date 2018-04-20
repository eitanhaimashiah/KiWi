#include "KiWiMapVLK.h"

namespace kiwi
{
    KiWiMapVLK::KiWiMapVLK() {
//    : kiwi(KiWi<Cell, Cell>(unique_ptr<ChunkCell>(new ChunkCell)))
        kiwi = KiWi<Cell, Cell>(unique_ptr<Chunk<Cell, Cell>>(new ChunkCell()));
    }

	int KiWiMapVLK::putIfAbsent(int k, int v)
	{
		kiwi.put(cellFromInt(k), cellFromInt(v));
		return 0;
	}

	int KiWiMapVLK::size()
	{
		return -1;
	}

	bool KiWiMapVLK::isEmpty()
	{
		return false;
	}

	int KiWiMapVLK::get(int k)
	{
		return cellToInt(kiwi.get(cellFromInt(k)));
	}

	int KiWiMapVLK::put(int k, int v)
	{
		kiwi.put(cellFromInt(k), cellFromInt(v));
		return 0;
	}

	int KiWiMapVLK::remove(int k)
	{
		kiwi.put(cellFromInt(k), cellFromInt(-1)); // TODO: Should replace -1 with nullptr (maybe value type should be ptr)
		return 0;
	}

	void KiWiMapVLK::putAll(map<int, int> map)
	{
		throw NotImplementedException();
	}

	int KiWiMapVLK::getRange(vector<int> &result, int min, int max)
	{
		throw NotImplementedException();
	}

	bool KiWiMapVLK::containsKey(int k)
	{
		return get(k) != -1; // TODO: Should replace -1 with nullptr (maybe value type should be ptr)
	}

	void KiWiMapVLK::clear()
	{
		kiwi = KiWi<Cell, Cell>(new ChunkCell);
	}

	set<int> *KiWiMapVLK::keySet()
	{
		throw NotImplementedException();
	}

	set<int> *KiWiMapVLK::values()
	{
		throw NotImplementedException();
	}

	set<pair<int, int>> KiWiMapVLK::entrySet()
	{
		throw NotImplementedException();
	}

	bool KiWiMapVLK::containsValue(int v)
	{
		return get(v) != -1; // TODO: Should replace -1 with nullptr (maybe value type should be ptr)
	}

	Cell KiWiMapVLK::cellFromInt(int n)
	{
		vector<char> b(4);

		// write int data as bytes into data-array
		b[0] = static_cast<char>(n >> 24);
		b[1] = static_cast<char>(n >> 16);
		b[2] = static_cast<char>(n >> 8);
		b[3] = static_cast<char>(n);

		return Cell(b, 0, 4); // TODO: propbablt it's not valid
	}

	int KiWiMapVLK::cellToInt(Cell *c)
	{
		if (c == nullptr)
		{
			return -1; // TODO: Should replace -1 with nullptr (maybe value type should be ptr)
		}

		vector<char> b = c->getBytes();
		int off = c->getOffset();

		int n = b[off + 0] << 24 | (b[off + 1] & 0xFF) << 16 | (b[off + 2] & 0xFF) << 8 | (b[off + 3] & 0xFF);

		return n;
	}

	void KiWiMapVLK::debugPrint()
	{
		kiwi.debugPrint();
	}
}
