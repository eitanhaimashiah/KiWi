#include <unordered_map>
#include <vector>

namespace kiwi
{


	/*
	 * Compositional map interface
	 * 
	 * @author Vincent Gramoli
	 *
	 */
	template<typename K, typename V>
	class CompositionalMap : public std::unordered_map<K, V>
	{

	public:
		static constexpr bool TRAVERSAL_COUNT = false;
		static constexpr bool STRUCT_MODS = false;

		class Vars
		{
		public:
			long long iteration = 0;
			long long getCount = 0;
			long long nodesTraversed = 0;
			long long structMods = 0;
		};

	public:
		static ThreadLocal<kiwi::CompositionalMap::Vars*> *const counts;

		virtual V putIfAbsent(K k, V v) = 0;

		virtual void clear() = 0;

		virtual int size() = 0;

		virtual int getRange(std::vector<K> &result, K min, K max) = 0;
	};


}
