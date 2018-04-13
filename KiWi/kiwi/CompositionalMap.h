#ifndef CompositionalMap_h
#define CompositionalMap_h

#include <map>
#include <vector>

using namespace std;

namespace kiwi
{
	/**
	 * Compositional map interface
	 */
	template<typename K, typename V, class Comparer = less<K>>
	class CompositionalMap : public map<K, V, Comparer>
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
		static thread_local Vars counts;

		virtual V putIfAbsent(K k, V v) = 0;

		virtual void clear() = 0;

		virtual int size() = 0;

		virtual int getRange(vector<K> &result, K min, K max) = 0;
	};
}

#endif /* CompositionalMap_h */
