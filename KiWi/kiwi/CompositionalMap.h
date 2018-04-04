#ifndef CompositionalMap_h
#define CompositionalMap_h

#include <unordered_map>
#include <vector>

using namespace std;

namespace kiwi
{
	/**
	 * Compositional map interface
	 */
	template<typename K, typename V>
	class CompositionalMap : public unordered_map<K, V>
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

		virtual int getRange(vector<K> &result, K min, K max) = 0;
	};
}

#endif /* CompositionalMap_h */
