#ifndef ChunkCell_h
#define ChunkCell_h

#include <map>
#include <vector>

#include "exceptionhelper.h"
#include "Cell.h"
#include "Chunk.h"

using namespace std;

namespace kiwi
{
    class ChunkCell : public Chunk<Cell, Cell>
	{
	private:
		static constexpr int DATA_SIZE = 100; // average # of BYTES of item in data array (guesstimate)

	public:
		ChunkCell();
        ~ChunkCell() {}
		ChunkCell(Cell *minKey, ChunkCell *creator);
		Chunk<Cell, Cell> *newChunk(Cell *minKey);


		Cell *readKey(int orderIndex);
		void *readData(int oi, int di);

		int copyValues(vector<void*> &result, int idx, int myVer, const Cell& min, const Cell& max, map<Cell, PutData>& items);

		int allocate(Cell *key, Cell *data);

		int allocateSerial(int key, Cell *data);
	};

}

#endif /* ChunkCell_h */
