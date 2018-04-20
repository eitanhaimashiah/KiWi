#ifndef ChunkInt_h
#define ChunkInt_h

#include <map>
#include <vector>
#include <atomic>
#include "Chunk.h"

using namespace std;

namespace kiwi
{
	class ChunkInt : public Chunk<int, int>
	{
        using Chunk<int, int>::Chunk;
        
	private:
        static atomic<int> nextChunk;
		static vector<ChunkInt> chunks;
	public:
		static void setPoolSize(int numChunks);
		static void initPool();

	private:
		static constexpr int DATA_SIZE = 1;    // average # of BYTES of item in data array (guesstimate)

	public:
		ChunkInt();
        ~ChunkInt() {}
        
		ChunkInt(int minKey, shared_ptr<ChunkInt> creator);
		Chunk<int, int> *newChunk(int minKey);

		int readKey(int orderIndex);
		void *readData(int oi, int di);

		int copyValues(vector<void*> &result, int idx, int myVer, const int min, const int max, map<int, PutData> const& items);

		int allocate(int key, int data);

		int allocateSerial(int key, int data);

	};
}

#endif /* ChunkInt_h */
