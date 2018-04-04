#ifndef ChunkCell_h
#define ChunkCell_h

#include <vector>
#include "exceptionhelper.h"
#include "Cell.h"
#include "Chunk.h"

using namespace std;

namespace kiwi
{
	class ChunkCell : public Chunk<Cell*, Cell*>
	{
	private:
		static constexpr int DATA_SIZE = 100; // average # of BYTES of item in data array (guesstimate)

	public:
		ChunkCell();
		ChunkCell(Cell *minKey, ChunkCell *creator);
		Chunk<Cell*, Cell*> *newChunk(Cell *minKey) override;


		Cell *readKey(int orderIndex) override;
		void *readData(int oi, int di) override;

		int copyValues(vector<void*> &result, int idx, int myVer, Cell *min, Cell *max, SortedMap<Cell*, PutData<Cell*, Cell*>*> *items) override;

		int allocate(Cell *key, Cell *data) override;

		int allocateSerial(int key, Cell *data) override;
	};

}

#endif /* ChunkCell_h */
