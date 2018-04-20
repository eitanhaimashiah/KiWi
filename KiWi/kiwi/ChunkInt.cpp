#include "ChunkInt.h"

namespace kiwi
{
    vector<ChunkInt> ChunkInt::chunks;
    atomic<int> ChunkInt::nextChunk (0);

	void ChunkInt::setPoolSize(int numChunks)
	{
		chunks = vector<ChunkInt>(numChunks);

	}

	void ChunkInt::initPool()
	{
		if (chunks.size() > 0)
		{
            nextChunk.store(0);
			for (int i = 0; i < chunks.size(); ++i)
			{
				chunks[i] = ChunkInt(0, nullptr);
			}
		}
	}

	ChunkInt::ChunkInt() : ChunkInt(numeric_limits<int>::min(), nullptr)
	{
	}

	ChunkInt::ChunkInt(int minKey, shared_ptr<Chunk<int, int>> creator) : Chunk<int,int>(minKey, DATA_SIZE, creator)
	{
	}

	Chunk<int, int> *ChunkInt::newChunk(int minKey)
	{
		if (chunks.empty())
		{
			return ChunkInt(minKey, this);
		}
		else
		{
			int next = nextChunk->getAndIncrement();
			ChunkInt *chunk = chunks[next];
			chunks[next] = nullptr;
			chunk->minKey = minKey;
			chunk->creator = this;
			return chunk;
		}
	}

	int ChunkInt::readKey(int orderIndex)
	{
		return get(orderIndex, OFFSET_KEY);
	}

	void *ChunkInt::readData(int oi, int di)
	{
		/*
		// get data - convert next 4 bytes to Integer data
		int data = dataArray[di] << 24 | (dataArray[di+1] & 0xFF) << 16 |
			(dataArray[di+2] & 0xFF) << 8 | (dataArray[di+3] & 0xFF);
		*/

		return dataArray[di];
	}

	int ChunkInt::copyValues(vector<void*> &result, int idx, int myVer, const int min, const int max, map<int, PutData> const& items)
	{
		int oi = 0;

		if (idx == 0)
		{
			oi = findFirst(min,myVer);
			if (oi == NONE)
			{
				return 0;
			}

		}
		else
		{
			oi = getFirst(myVer);
		}

		int sortedSize = sortedCount*ORDER_SIZE;
		int prevKey = NONE;
		int currKey = NONE;
		int maxKey = max;

		int dataStart = get(oi,OFFSET_DATA);
		int dataEnd = dataStart - 1;
		int itemCount = 0;
		int oiPrev = NONE;
		int prevDataId = NONE;
		int currDataId = dataStart;


		bool isFirst = dataStart > 0 ? true : false;

		while (oi != NONE)
		{

			if (currKey > max)
			{
				break;
			}

			if (isFirst || ((oiPrev < sortedSize) && (prevKey != currKey) && currKey <= maxKey && (prevDataId + DATA_SIZE == currDataId) && (getVersion(oi) <= myVer)))
			{
				if (isFirst)
				{
					dataEnd++;
					isFirst = false;
				}
				else
				{
					dataEnd = currDataId < 0 ? dataEnd : dataEnd + 1;
				}

				oiPrev = oi;
				oi = get(oi, OFFSET_NEXT);

				prevKey = currKey;
				prevDataId = currDataId;
				currKey = get(oi,OFFSET_KEY);
				currDataId = get(oi,OFFSET_DATA);

			}
			else
			{
				// copy continuous range of data
				int itemsToCopy = dataEnd - dataStart + 1;

				if (itemsToCopy == 1)
				{
					result[idx + itemCount] = dataArray[dataStart];
				}
				else if (itemsToCopy > 1)
				{
					System::arraycopy(dataArray, dataStart, result, idx + itemCount, itemsToCopy);
				}

				itemCount += itemsToCopy;

				// find next item to start copy interval
				while (oi != NONE)
				{

					if (currKey > max)
					{
						return itemCount;
					}

					// if in a valid version, and a different key - found next item
					if ((prevKey != currKey) && (getVersion(oi) <= myVer))
					{
						if (currDataId < 0)
						{
							// the value is NULL, the item was removed -- skip it
							prevKey = currKey;

						}
						else
						{
							break;
						}
					}

					// otherwise proceed to next
					oiPrev = oi;
					oi = get(oi, OFFSET_NEXT);
					currKey = get(oi,OFFSET_KEY);

					prevDataId = currDataId;
					currDataId = get(oi,OFFSET_DATA);
				}

				if (oi == NONE)
				{
					return itemCount;
				}

				dataStart = currDataId; //get(oi, OFFSET_DATA);
				dataEnd = dataStart - 1;
				isFirst = true;
			}

		}

		int itemsToCopy = dataEnd - dataStart + 1;
		System::arraycopy(dataArray,dataStart, result, idx + itemCount, itemsToCopy);

		itemCount += itemsToCopy;



		return itemCount;
	}

	int ChunkInt::allocate(int key, int data)
	{
		// allocate items in order and data array => data-array only contains int-sized data
		int oi = baseAllocate(data == nullptr ? 0 : DATA_SIZE);

		if (oi >= 0)
		{
			// write integer key into (int) order array at correct offset
			set(oi, OFFSET_KEY, static_cast<int>(key));

			// get data index
			if (data != nullptr)
			{
				int di = get(oi, OFFSET_DATA);
				dataArray[di] = data;
			}

		}

		// return order-array index (can be used to get data-array index)
		return oi;
	}

	int ChunkInt::allocateSerial(int key, int data)
	{
		// allocate items in order and data array => data-array only contains int-sized data
		int oi = baseAllocateSerial(data == nullptr ? 0 : DATA_SIZE);

		if (oi >= 0)
		{
			// write integer key into (int) order array at correct offset
			set(oi, OFFSET_KEY, key);

			if (data != nullptr)
			{
				int di = get(oi, OFFSET_DATA);
				dataArray[di] = data;
			}
		}

		// return order-array index (can be used to get data-array index)
		return oi;
	}
}
