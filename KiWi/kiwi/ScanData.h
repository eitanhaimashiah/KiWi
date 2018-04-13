#ifndef ScanData_h
#define ScanData_h

#include <atomic>

using namespace std;

namespace kiwi
{
    /* Thread data class for Scan operations */
    template<typename K>
    class ScanData
    {
    public:
        // TODO: Taken from the Chunk class, think of better idea...
        static constexpr int NONE = 0;;
        
        ScanData(K min, K max) : min(min), max(max), version(NONE) {}
        
        atomic<int> const version;
        const K min;
        const K max;
        
        static bool compare(ScanData<K> &o1, ScanData<K> &o2) {
            return o2.version.load() - o1.version.load();
        }
    };
}

#endif /* ScanData_h */
