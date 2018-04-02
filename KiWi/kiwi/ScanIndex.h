#include "ThreadData.h"
#include <vector>
#include <limits>
#include <type_traits>

//JAVA TO C++ CONVERTER NOTE: Forward class declarations:
namespace kiwi { class ThreadData; }
namespace kiwi { template<typename Ktypename V>class ScanData; }

namespace kiwi
{

	using kiwi::ThreadData::ScanData;


	/// <summary>
	/// Created by dbasin on 11/30/15.
	/// </summary>
	template<typename K>
	class ScanIndex
	{
		static_assert(std::is_base_of<Comparable<? super K>, K>::value, L"K must inherit from Comparable<? super K>");

	private:
		std::vector<int> scanVersions;
		std::vector<int> fromKeys;
		std::vector<int> toKeys;

		//private ScanData[] scans;
	public:
		bool isFirst = false;
		bool isOutOfRange = false;

	private:
		int index = 0;
		int numOfVersions = 0;
		Integer min = 0;
		Integer max = 0;
		int currKey = 0;

	public:
		ScanIndex(std::vector<ScanData*> &scans, int currVersion, K minKey, K maxKey)
		{
			//this.scans = scans;
			this->scanVersions = std::vector<int>(scans.size());
			this->fromKeys = std::vector<int>(scans.size());
			this->toKeys = std::vector<int>(scans.size());

			this->numOfVersions = 0;

			this->min = (minKey != nullptr) ? static_cast<Integer>(minKey): std::numeric_limits<int>::min();
			this->max = (maxKey != nullptr) ? static_cast<Integer>(maxKey): std::numeric_limits<int>::max();

			//noinspection Since15
			ComparatorAnonymousInnerClass tempVar(this);
			scans.sort(&tempVar);

			//for(int i = 0; i < scans.length; ++i)
			for (auto sd : scans)
			{
				if (sd == nullptr)
				{
					continue;
				}
				if (sd->max->compareTo(min) < 0)
				{
					continue;
				}
				if (sd->min->compareTo(max) > 0)
				{
					continue;
				}

				scanVersions[numOfVersions] = sd->version->get();
				fromKeys[numOfVersions] = static_cast<int>(sd->min);
				toKeys[numOfVersions] = static_cast<int>(sd->max);
				numOfVersions++;
			}



	/*
	        Arrays.sort(scanVersions);
	        int size = Math.min(numOfVersions,scanVersions.length/2);
	
	        // mirror the array
	        for( int i = 0; i < size ; ++i )
	        {
	            int temp = scanVersions[i];
	            scanVersions[i] = scanVersions[scanVersions.length - i - 1];
	            scanVersions[scanVersions.length - i - 1] = temp;
	        }
	*/
			reset(-1);
		}

	private:
		class ComparatorAnonymousInnerClass : public Comparator<ScanData*>
		{
		private:
			ScanIndex<K*> *outerInstance;

		public:
			ComparatorAnonymousInnerClass(ScanIndex<K*> *outerInstance);

			virtual int compare(ScanData *o1, ScanData *o2);
		};

	public:
		void reset(int key)
		{
			index = -1;
			isFirst = true;
			currKey = key;
		}

		/// <summary>
		///*
		/// 
		/// </summary>
		/// <param name="version"> -- we assume that version > 0
		/// @return </param>
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
