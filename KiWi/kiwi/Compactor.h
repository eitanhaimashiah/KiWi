#ifndef Compactor_h
#define Compactor_h

#include <vector>
#include "Chunk.h"

using namespace std;

namespace kiwi
{
	template<typename K, typename V>
	class Compactor
	{

	public:
		virtual vector<Chunk<K, V>*> compact(vector<Chunk<K, V>*> &frozenChunks, ScanIndex<K> *scanIndex) = 0;
	};

}

#endif /* Compactor_h */
