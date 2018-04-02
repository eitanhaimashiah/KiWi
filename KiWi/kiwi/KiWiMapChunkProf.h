#include "KiWiMap.h"
#include <string>
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "tangible_filesystem.h"

//JAVA TO C++ CONVERTER NOTE: Forward class declarations:
namespace kiwi { class DebugStats; }

namespace kiwi
{


	/// <summary>
	/// Created by dbasin on 1/24/16.
	/// </summary>
	class KiWiMapChunkProf : public KiWiMap
	{

	private:
		AtomicInteger *const opsDone = new AtomicInteger(0);
		const int opsPerSnap = Parameters::range / 20;

		int snapCount = 0;
		int totalOps = 0;

		static const std::wstring profDirPath;
		static const std::wstring profFileName;

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