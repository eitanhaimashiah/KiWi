#ifndef ChunkIterator_h
#define ChunkIterator_h

namespace kiwi
{
	template<typename K, typename V>
	class ChunkIterator
	{
	public:
		virtual Chunk<K, V> *getNext(Chunk<K, V> *chunk) = 0;
		virtual Chunk<K, V> *getPrev(Chunk<K, V> *chunk) = 0;
	};

}

#endif /* ChunkIterator_h */
