#ifndef ThreadData_h
#define ThreadData_h

#include <atomic>
#include "Chunk.h"

namespace kiwi
{
	class ThreadData
	{
		
	public:
        // Thread data class for Scan operations
		template<typename K>
		class ScanData;

        // Thread data class for Put operations
        template<typename K, typename V>
		class PutData;
	};

    template<typename K>
    class ThreadData::ScanData
	{
	public:
		ScanData(K min, K max) : min(min), max(max) {}

        std::atomic<int> const version (Chunk::NONE);
		const K min;
		const K max;
        
        static bool compare(ScanData<K> &o1, ScanData<K> &o2) {
            return o2.version.load() - o1.version.load();
        }

	};
    
    template<typename K, typename V>
    class ThreadData::PutData
	{
	public:
		const int orderIndex;

		PutData(int orderIndex) : orderIndex(orderIndex) {}
	};
}

#endif /* ThreadData */
