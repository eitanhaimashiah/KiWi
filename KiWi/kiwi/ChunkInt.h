#ifndef ChunkInt_h
#define ChunkInt_h

#include <vector>
#include <atomic>
#include "Chunk.h"

namespace kiwi
{
	class ChunkInt : public Chunk<int, int>
	{
	private:
        static std::atomic<int> nextChunk;
		static std::vector<ChunkInt> chunks;
	public:
		static void setPoolSize(int numChunks);
		static void initPool();

	private:
		static constexpr int DATA_SIZE = 1;    // average # of BYTES of item in data array (guesstimate)

	public:
		ChunkInt();
		ChunkInt(int minKey, ChunkInt *creator);
		Chunk<Integer, Integer> *newChunk(Integer minKey) override;

		int readKey(int orderIndex) override;
		void *readData(int oi, int di) override;

		int copyValues(std::vector<void*> &result, int idx, int myVer, Integer min, Integer max, SortedMap<Integer, ThreadData::PutData<Integer, Integer>*> *items) override;

		int allocate(Integer key, Integer data) override;

		int allocateSerial(int key, Integer data) override;

	};
}

#endif /* ChunkInt_h */
