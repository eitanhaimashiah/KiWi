#ifndef Compactor_h
#define Compactor_h

#include <vector>
#include "Chunk.h"

namespace kiwi
{
	template<typename K, typename V>
	class Compactor
	{

	public:
		virtual std::vector<Chunk<K, V>*> compact(std::vector<Chunk<K, V>*> &frozenChunks, ScanIndex<K> *scanIndex) = 0;
	};

}

#endif /* Compactor_h */
