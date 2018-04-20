#ifndef ChunkIterator_h
#define ChunkIterator_h

// Class forward declaration
namespace kiwi { template<typename K, typename V, class Comparer> class Chunk; }

using namespace std;

namespace kiwi
{
	template<typename K, typename V, class Comparer = less<K>>
	class ChunkIterator
	{
	public:
		virtual Chunk<K, V, Comparer>* getNext(Chunk<K, V, Comparer> *chunk) = 0;
		virtual Chunk<K, V, Comparer>* getPrev(Chunk<K, V, Comparer> *chunk) = 0;
	};

}

#endif /* ChunkIterator_h */
