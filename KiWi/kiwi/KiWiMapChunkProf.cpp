#include "KiWiMapChunkProf.h"

namespace kiwi
{

const string KiWiMapChunkProf::profDirPath = "./../output/data";
const string KiWiMapChunkProf::profFileName = "chunkStat.csv";

    KiWiMapChunkProf::KiWiMapChunkProf() :
    opsDone(0), opsPerSnap(Parameters::range / 20), snapCount(0), totalOps(0), writer(nullptr), isUnderSnapshot(false) {}
    
	void KiWiMapChunkProf::trySnapshot()
	{

        int currOps = opsDone.fetch_add(1);

		if (currOps >= opsPerSnap)
		{
            unique_lock<mutex> lock(_mutex);
            currOps = opsDone.load();
            
            if (currOps < opsPerSnap)
            {
                return;
            }
            
            isUnderSnapshot = true;
            
            totalOps += currOps;
            snapCount++;
            
            printStatsLine(kiwi.calcChunkStatistics());
            
            opsDone.store(0);
            
            isUnderSnapshot = false;
            unique_lock<mutex> unlock(_mutex);
		}
	}

	void KiWiMapChunkProf::printStatsHeader()
	{
		try
		{
			writer->write("FillType\tOps\tTotalItems\tChunks\tSorted\tChunkItems\tOccupied\tDuplicates\tNulls\tRemoved\tKeyJumps\tValJumps");
			writer->newLine();

		}
		catch (const IOException &e)
		{
			e->printStackTrace();
		}
	}

	void KiWiMapChunkProf::printStatsLine(DebugStats *ds)
	{
		StringBuilder *sb = new StringBuilder();

		wchar_t delimeter = L'\t';
		sb->append(Parameters::fillType);
		sb->append(delimeter);

		sb->append(totalOps)->append(delimeter)->append(ds->itemCount)->append(delimeter)->append(ds->chunksCount)->append(delimeter)->append(ds->sortedCells / ds->chunksCount)->append(delimeter)->append(ds->itemCount / ds->chunksCount)->append(delimeter)->append(ds->occupiedCells / ds->chunksCount)->append(delimeter)->append(ds->duplicatesCount / ds->chunksCount)->append(delimeter)->append(ds->nulItemsCount / ds->chunksCount)->append(delimeter)->append(ds->removedItems / ds->chunksCount)->append(delimeter)->append(ds->jumpKeyCount / ds->chunksCount)->append(delimeter)->append(ds->jumpValCount / ds->chunksCount);

		try
		{
			writer->write(sb->toString());
			writer->newLine();
			writer->flush();
		}
		catch (const IOException &e)
		{
			e->printStackTrace();
		}
	}

	KiWiMapChunkProf::KiWiMapChunkProf() : KiWiMap()
	{
		FileSystem::createDirectory(profDirPath);

		outFile = new File(profDirPath + L"/" + profFileName);

	   // if(outFile.exists())
		 //   outFile.delete();

		try
		{
			outFile->createNewFile();
		}
		catch (const IOException &e)
		{
			e->printStackTrace();
		}



		try
		{
			FileWriter tempVar(outFile->getCanonicalPath());
			writer = new BufferedWriter(&tempVar);
			printStatsHeader();

		}
		catch (const IOException &ioe)
		{
			ioe->printStackTrace();
		}

	}

	int KiWiMapChunkProf::put(int key, int val)
	{
		// busy wait here
		while (isUnderSnapshot)
		{
			;
		}

		KiWiMap::emplace(key, val);

		trySnapshot();

		return nullptr;

	}

	int KiWiMapChunkProf::remove(void *key)
	{
		// busy wait here
		while (isUnderSnapshot)
		{
			;
		}

		KiWiMap::erase(key);

		trySnapshot();

		return nullptr;
	}
}
