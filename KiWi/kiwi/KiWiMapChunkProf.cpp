

#include "KiWiMapChunkProf.h"
#include "DebugStats.h"

namespace kiwi
{

const wstring KiWiMapChunkProf::profDirPath = wstring(L"./../output/data");
const wstring KiWiMapChunkProf::profFileName = L"chunkStat.csv";

	void KiWiMapChunkProf::trySnapshot()
	{

		int currOps = opsDone->getAndIncrement();

		if (currOps >= opsPerSnap)
		{
//JAVA TO C++ CONVERTER TODO TASK: Multithread locking is not converted to native C++ unless you choose one of the options on the 'Miscellaneous Options' dialog:
			synchronized(this)
			{
				currOps = opsDone->get();

				if (currOps < opsPerSnap)
				{
					return;
				}

				isUnderSnapshot = true;

				totalOps += currOps;
				snapCount++;

				printStatsLine(kiwi->calcChunkStatistics());

				opsDone->set(0);

				isUnderSnapshot = false;

			}
		}
	}

	void KiWiMapChunkProf::printStatsHeader()
	{
		try
		{
			writer->write(L"FillType\tOps\tTotalItems\tChunks\tSorted\tChunkItems\tOccupied\tDuplicates\tNulls\tRemoved\tKeyJumps\tValJumps");
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

	Integer KiWiMapChunkProf::put(Integer key, Integer val)
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

	Integer KiWiMapChunkProf::remove(void *key)
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
