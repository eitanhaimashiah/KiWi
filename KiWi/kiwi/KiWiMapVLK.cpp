#include "KiWiMapVLK.h"
#include "KiWi.h"

namespace kiwi
{
	using sun::reflect::generics::reflectiveObjects::NotImplementedException;

	KiWiMapVLK::KiWiMapVLK()
	{
		ChunkCell tempVar();
		this->kiwi = new KiWi<Cell*, Cell*>(&tempVar);
	}

	Integer KiWiMapVLK::putIfAbsent(Integer k, Integer v)
	{

		kiwi->put(cellFromInt(k), cellFromInt(v));
		return nullptr;
	}

	int KiWiMapVLK::size()
	{
		return -1;
	}

	bool KiWiMapVLK::isEmpty()
	{
		return false;
	}

	Integer KiWiMapVLK::get(void *o)
	{
		return cellToInt(kiwi->get(cellFromInt(static_cast<Integer>(o))));
	}

	Integer KiWiMapVLK::put(Integer k, Integer v)
	{
		kiwi->put(cellFromInt(k), cellFromInt(v));
		return nullptr;
	}

	Integer KiWiMapVLK::remove(void *o)
	{
		kiwi->put(cellFromInt(static_cast<Integer>(o)), nullptr);
		return nullptr;
	}

template<typename T1, typename T1>
//JAVA TO C++ CONVERTER TODO TASK: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public void putAll(java.util.Map<? extends Integer, ? extends Integer> map)
	void KiWiMapVLK::putAll(std::unordered_map<T1> map)
	{
		throw NotImplementedException();
	}

	int KiWiMapVLK::getRange(std::vector<Integer> &result, Integer min, Integer max)
	{
		throw NotImplementedException();
	}

	bool KiWiMapVLK::containsKey(void *o)
	{
		return get(cellFromInt(static_cast<Integer>(o))) != nullptr;
	}

	void KiWiMapVLK::clear()
	{
		//this.kiwi.debugPrint();
		ChunkCell tempVar();
		this->kiwi = new KiWi<Cell*, Cell*>(&tempVar);
	}

	Set<Integer> *KiWiMapVLK::keySet()
	{
		throw NotImplementedException();
	}

	Collection<Integer> *KiWiMapVLK::values()
	{
		throw NotImplementedException();
	}

	Set<Entry<Integer, Integer>*> *KiWiMapVLK::entrySet()
	{
		throw NotImplementedException();
	}

	bool KiWiMapVLK::containsValue(void *o)
	{
		return (get(cellFromInt(static_cast<Integer>(o))) != nullptr);
	}

	Cell *KiWiMapVLK::cellFromInt(Integer n)
	{
		std::vector<char> b(4);

		// write int data as bytes into data-array
		b[0] = static_cast<char>(static_cast<int>(static_cast<unsigned int>(n) >> 24));
		b[1] = static_cast<char>(static_cast<int>(static_cast<unsigned int>(n) >> 16));
		b[2] = static_cast<char>(static_cast<int>(static_cast<unsigned int>(n) >> 8));
		b[3] = static_cast<char>(n.intValue());

		return new Cell(b, 0, 4);
	}

	Integer KiWiMapVLK::cellToInt(Cell *c)
	{
		if (c == nullptr)
		{
			return nullptr;
		}

		std::vector<char> b = c->getBytes();
		int off = c->getOffset();

		int n = b[off + 0] << 24 | (b[off + 1] & 0xFF) << 16 | (b[off + 2] & 0xFF) << 8 | (b[off + 3] & 0xFF);

		return n;
	}

	void KiWiMapVLK::debugPrint()
	{
		kiwi->debugPrint();
	}
}