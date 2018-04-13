#ifndef Compactor_h
#define Compactor_h

#include <vector>
#include "Chunk.h"

using namespace std;

namespace kiwi
{
	template<typename K, typename V, class Comparer = less<K>>
	class Compactor
	{

	public:
		virtual vector<Chunk<K, V, Comparer>*> compact(vector<Chunk<K, V, Comparer>*> &frozenChunks, ScanIndex<K> *scanIndex) = 0;
	};

}

#endif /* Compactor_h */
