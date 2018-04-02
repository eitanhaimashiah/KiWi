#ifndef GALOIS_WORKLIST_KIWI_THREADDATA_H
#define GALOIS_WORKLIST_KIWI_THREADDATA_H

#include <atomic>
#include "Chunk.h"

namespace kiwi
{
	class ThreadData
	{
		
	public:
        // Thread data class for Scan operations
		template<typename K, typename V>
		class ScanData;

        // Thread data class for Put operations
        template<typename K, typename V>
		class PutData;
	};

    template<typename K, typename V>
    class ThreadData::ScanData
	{
	public:
		virtual ~ScanData()
		{
			delete version;
		}

		ScanData(K min, K max) : min(min), max(max) {}

        std::atomic<int> *const version (Chunk::NONE);
		const K min;
		const K max;

	};
    
    template<typename K, typename V>
    class ThreadData::PutData
	{
	public:
		const int orderIndex;

		PutData(int orderIndex) : orderIndex(orderIndex) {}
	};
}

#endif
