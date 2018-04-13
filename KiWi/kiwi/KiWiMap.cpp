#include "KiWiMap.h"

namespace kiwi
{
    bool KiWiMap::SupportScan = true;
    int KiWiMap::RebalanceSize = 2;

	KiWiMap::KiWiMap()
	{
		ChunkInt::initPool();
        KiWi<int,int>::RebalanceSize = RebalanceSize;
		ChunkInt tempVar();
		kiwi = new KiWi<Integer,Integer>(&tempVar, SupportScan);
	}

	Integer KiWiMap::putIfAbsent(Integer k, Integer v)
	{
		kiwi->put(k, v);
		return nullptr; // can implement return value but not necessary
	}

	int KiWiMap::size()
	{
		return -1;
	}

	bool KiWiMap::isEmpty()
	{
		return false;
	}

	Integer KiWiMap::get(void *o)
	{
		return kiwi->get(static_cast<Integer>(o));
	}

	Integer KiWiMap::put(Integer k, Integer v)
	{
		kiwi->put(k, v);
		return nullptr;
	}

	Integer KiWiMap::remove(void *o)
	{
		kiwi->put(static_cast<Integer>(o), nullptr);
		return nullptr;
	}

	int KiWiMap::getRange(vector<Integer> &result, Integer min, Integer max)
	{
		return kiwi->scan(result,min,max);
		/*
		    	Iterator<Integer> iter = kiwi.scan(min, max);
		    	int i;
		    	
		    	for (i = 0; (iter.hasNext()) && (i < result.length); ++i)
		    	{
		    		result[i] = iter.next();
		    	}
		    	
		    	return i;
		*/
	}

template<typename T1, typename T1>
//JAVA TO C++ CONVERTER TODO TASK: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public void putAll(java.util.Map<? extends Integer, ? extends Integer> map)
	void KiWiMap::putAll(unordered_map<T1> map)
	{
		for (auto key : map)
		{
			kiwi->put(key.first, map[key.first]);
		}
	}

	bool KiWiMap::containsKey(void *o)
	{
		return get(o) != nullptr;
	}

	void KiWiMap::clear()
	{
		//this.kiwi.debugPrint();
		ChunkInt::initPool();
		ChunkInt tempVar();
		this->kiwi = new KiWi<Integer,Integer>(&tempVar, SupportScan);
	}

	Set<Integer> *KiWiMap::keySet()
	{
		throw NotImplementedException();
	}

	Collection<Integer> *KiWiMap::values()
	{
		throw NotImplementedException();
	}

	Set<Entry<Integer, Integer>*> *KiWiMap::entrySet()
	{
		throw NotImplementedException();
	}

	bool KiWiMap::containsValue(void *o)
	{
		throw NotImplementedException();
	}

	void KiWiMap::compactAllSerial()
	{
		kiwi->compactAllSerial();
	}

	int KiWiMap::debugCountDups()
	{
		return kiwi->debugCountDups();
	}

	int KiWiMap::debugCountKeys()
	{
		return kiwi->debugCountKeys();
	}

	void KiWiMap::debugPrint()
	{
		kiwi->debugPrint();
	}

	int KiWiMap::debugCountDuplicates()
	{
		return kiwi->debugCountDuplicates();
	}

	int KiWiMap::debugCountChunks()
	{
		return 0;
	}

	void KiWiMap::calcChunkStatistics()
	{
		kiwi->calcChunkStatistics();
	}
}
