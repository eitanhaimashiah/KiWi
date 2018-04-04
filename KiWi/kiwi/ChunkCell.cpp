

#include "ChunkCell.h"
#include "Cell.h"

namespace kiwi
{
	ChunkCell::ChunkCell() : ChunkCell(Cell::Empty, nullptr)
	{
	}

	ChunkCell::ChunkCell(Cell *minKey, ChunkCell *creator) : Chunk<Cell, Cell>(minKey, DATA_SIZE, creator)
	{
	}

	Chunk<Cell*, Cell*> *ChunkCell::newChunk(Cell *minKey)
	{
		return new ChunkCell(minKey->clone(), this);
	}

	Cell *ChunkCell::readKey(int orderIndex)
	{
		throw NotImplementedException();
	}

	void *ChunkCell::readData(int oi, int di)
	{

		throw NotImplementedException();
	}

	int ChunkCell::copyValues(std::vector<void*> &result, int idx, int myVer, Cell *min, Cell *max, SortedMap<Cell*, ThreadData::PutData<Cell*, Cell*>*> *items)
	{
		throw NotImplementedException();
	}

	int ChunkCell::allocate(Cell *key, Cell *data)
	{
		throw NotImplementedException();
	}

	int ChunkCell::allocateSerial(int key, Cell *data)
	{
		throw NotImplementedException();
	}
}
