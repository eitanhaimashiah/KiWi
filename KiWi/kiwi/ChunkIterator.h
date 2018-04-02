#include <type_traits>

namespace kiwi
{


	/// <summary>
	/// Created by dbasin on 11/25/15.
	/// </summary>
	template<typename K, typename V>
	class ChunkIterator
	{
		static_assert(std::is_base_of<Comparable<? super K>, K>::value, L"K must inherit from Comparable<? super K>");

	public:
		virtual Chunk<K, V> *getNext(Chunk<K, V> *chunk) = 0;
		virtual Chunk<K, V> *getPrev(Chunk<K, V> *chunk) = 0;
	};

}
