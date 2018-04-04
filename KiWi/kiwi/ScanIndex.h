#ifndef ScanIndex_h
#define ScanIndex_h

#include <vector>
#include <algorithm>
#include <limits>
#include "ThreadData.h"

namespace kiwi
{
	template<typename K>
	class ScanIndex
	{
	private:
		std::vector<int> scanVersions;
		std::vector<int> fromKeys;
		std::vector<int> toKeys;
        
        int index = 0;
        int numOfVersions = 0;
        int min = 0;
        int max = 0;
        int currKey = 0;
        
    public:
		bool isFirst = false;
		bool isOutOfRange = false;

		ScanIndex(std::vector<ThreadData::ScanData<K>> &scans, int currVersion, K minKey, K maxKey)
		{
			scanVersions = std::vector<int>(scans.size());
			fromKeys = std::vector<int>(scans.size());
			toKeys = std::vector<int>(scans.size());

			numOfVersions = 0;

			min = (minKey != nullptr) ? static_cast<int>(minKey): std::numeric_limits<int>::min();
			max = (maxKey != nullptr) ? static_cast<int>(maxKey): std::numeric_limits<int>::max();

            std::sort(scans.begin(), scans.end(), ThreadData::ScanData<K>::compare);

			for (auto sd : scans)
			{
				if (sd == nullptr) continue;
				if (static_cast<int>(sd.max) < static_cast<int>(sd.min)) continue; // Eitan: They used the generic compareTo function.

                scanVersions[numOfVersions] = sd.version.load();
				fromKeys[numOfVersions] = static_cast<int>(sd.min);
				toKeys[numOfVersions] = static_cast<int>(sd.max);
				numOfVersions++;
			}
            
			reset(-1);
		}

		void reset(int key)
		{
			index = -1;
			isFirst = true;
			currKey = key;
		}

        /***
         * @param version -- we assume that version > 0
         */
		bool shouldKeep(int version)
		{
			//always save the first provided version.
			if (isFirst)
			{
				return true;
			}
			if (index >= numOfVersions)
			{
				return false;
			}

			if (fromKeys[index] > currKey)
			{
				return false;
			}
			if (toKeys[index] < currKey)
			{
				return false;
			}

			return scanVersions[index] >= version;

		}

		void savedVersion(int version)
		{
			isFirst = false;
			index++;
		}
	};

}

#endif /* ScanIndex_h */

