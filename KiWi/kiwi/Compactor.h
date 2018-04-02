#include <vector>
#include <type_traits>

namespace kiwi
{

	/// <summary>
	/// Created by dbasin on 12/1/15.
	/// </summary>
	template<typename K, typename V>
	class Compactor
	{
		static_assert(std::is_base_of<Comparable<K>, K>::value, L"K must inherit from Comparable<K>");

	public:
		virtual std::vector<Chunk<K, V>*> compact(std::vector<Chunk<K, V>*> &frozenChunks, ScanIndex<K> *scanIndex) = 0;
	};

}
