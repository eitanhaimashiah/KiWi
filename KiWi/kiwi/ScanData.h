#ifndef ScanData_h
#define ScanData_h

#include <atomic>
#include "Chunk.h"

using namespace std;

namespace kiwi
{
    /* Thread data class for Scan operations */
    template<typename K>
    class ScanData
    {
    public:
        ScanData(K min, K max) : min(min), max(max), version(Chunk<int,int>::NONE) {}
        
        atomic<int> const version;
        const K min;
        const K max;
        
        static bool compare(ScanData<K> &o1, ScanData<K> &o2) {
            return o2.version.load() - o1.version.load();
        }
    };
}

#endif /* ScanData_h */
