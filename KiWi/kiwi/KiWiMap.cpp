#include "KiWiMap.h"

namespace kiwi
{
    bool KiWiMap::SupportScan = true;
    int KiWiMap::RebalanceSize = 2;

	KiWiMap::KiWiMap()
	{
		ChunkInt::initPool();
        KiWi<int,int>::RebalanceSize = RebalanceSize;
        unique_ptr<Chunk<int, int>> p = unique_ptr<Chunk<int, int>>(new ChunkInt);
		kiwi = KiWi<int,int>(, SupportScan);
	}

	int KiWiMap::putIfAbsent(int k, int v)
	{
		kiwi.put(k, v);
		return 0; // can implement return value but not necessary
	}

	int KiWiMap::size()
	{
		return -1;
	}

	bool KiWiMap::isEmpty()
	{
		return false;
	}

	int* KiWiMap::get(int k)
	{
		return kiwi.get(k);
	}

	int KiWiMap::put(int k, int v)
	{
		kiwi.put(k, v);
		return 0;
	}

	int KiWiMap::remove(int k)
	{
		kiwi.put(k, -1); // TODO: Should replace -1 with nullptr (maybe value type should be ptr)
		return 0;
	}

	int KiWiMap::getRange(vector<int> &result, int min, int max)
	{
		return kiwi.scan(result,min,max);
	}

	void KiWiMap::putAll(map<int, int> map)
	{
		for (auto key : map)
		{
			kiwi.put(key.first, map[key.first]);
		}
	}

	bool KiWiMap::containsKey(int k)
	{
		return get(k) != nullptr;
	}

	void KiWiMap::clear()
	{
		//this.kiwi.debugPrint();
		ChunkInt::initPool();
		ChunkInt tempVar();
		this->kiwi = new KiWi<int,int>(&tempVar, SupportScan);
	}

	set<int> *KiWiMap::keySet()
	{
		throw NotImplementedException();
	}

	set<int> *KiWiMap::values()
	{
		throw NotImplementedException();
	}

	set<pair<int, int>> KiWiMap::entrySet()
	{
		throw NotImplementedException();
	}

	bool KiWiMap::containsValue(int v)
	{
		throw NotImplementedException();
	}

	void KiWiMap::compactAllSerial()
	{
		kiwi.compactAllSerial();
	}

	int KiWiMap::debugCountDups()
	{
		return kiwi.debugCountDups();
	}

	int KiWiMap::debugCountKeys()
	{
		return kiwi.debugCountKeys();
	}

	void KiWiMap::debugPrint()
	{
		kiwi.debugPrint();
	}

	int KiWiMap::debugCountDuplicates()
	{
		return kiwi.debugCountDuplicates();
	}

	int KiWiMap::debugCountChunks()
	{
		return 0;
	}

	void KiWiMap::calcChunkStatistics()
	{
		kiwi.calcChunkStatistics();
	}
}
