#ifndef KiWiMapChunkProf_h
#define KiWiMapChunkProf_h

#include <string>
#include <mutex>

#include "KiWiMap.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "tangible_filesystem.h"
#include "Parameters.h"
#include "DebugStats.h"

using namespace std;

namespace kiwi
{
	class KiWiMapChunkProf : public KiWiMap
	{

	private:
        atomic<int> opsDone;
        const int opsPerSnap;

        int snapCount;
        int totalOps;

		static const string profDirPath;
		static const string profFileName;

        mutex _mutex;
	public:
        BufferedWriter *writer;

	private:
		File *outFile;

		bool isUnderSnapshot; // TODO: originally, it has volatile, what's the equivalent in C++?

	private:
		void trySnapshot();

		void printStatsHeader();

		void printStatsLine(DebugStats *ds);

	public:
		KiWiMapChunkProf();
		int put(int key, int val) override;

		int remove(void *key) override;
	};

}

#endif /* KiWiMapChunkProf_h */
