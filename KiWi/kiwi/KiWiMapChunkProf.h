#ifndef KiWiMapChunkProf_h
#define KiWiMapChunkProf_h

#include "KiWiMap.h"
#include <string>
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "tangible_filesystem.h"

//JAVA TO C++ CONVERTER NOTE: Forward class declarations:
namespace kiwi { class DebugStats; }

using namespace std;

namespace kiwi
{
	class KiWiMapChunkProf : public KiWiMap
	{

	private:
		AtomicInteger *const opsDone = new AtomicInteger(0);
		const int opsPerSnap = Parameters::range / 20;

		int snapCount = 0;
		int totalOps = 0;

		static const wstring profDirPath;
		static const wstring profFileName;

	public:
		BufferedWriter *writer = nullptr;

	private:
		File *outFile;

//JAVA TO C++ CONVERTER TODO TASK: 'volatile' has a different meaning in C++:
//ORIGINAL LINE: private volatile boolean isUnderSnapshot = false;
		bool isUnderSnapshot = false;

	public:
		virtual ~KiWiMapChunkProf()
		{
			delete opsDone;
			delete writer;
			delete outFile;
		}

	private:
		void trySnapshot();

		void printStatsHeader();

		void printStatsLine(DebugStats *ds);

	public:
		KiWiMapChunkProf();
		Integer put(Integer key, Integer val) override;

		Integer remove(void *key) override;
	};

}

#endif /* KiWiMapChunkProf_h */
