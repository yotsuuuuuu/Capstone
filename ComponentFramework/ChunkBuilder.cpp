#include "ChunkBuilder.h"
#include "WorldConstants.h"

void ChunkBuilder::generateChunk(Chunk& chunk, const TerrainNoise& noise)
{
	// heightmap is specific to this chunk
	float heightmap[CHUNK_SIZE * CHUNK_SIZE]; // 1D mapped array to hold height values but it is an expression of a 64 x 64 grid

	for (int x = 0; x < CHUNK_SIZE; x++) { //  for each vertex in the chunk

		for (int z = 0; z < CHUNK_SIZE; z++) {

			//calculate world position (starting at 0,0 for the current chunk)
			float worldX = chunk.chunkCoord.x * (CHUNK_WORLD_SIZE) + x;
			float worldZ = chunk.chunkCoord.y * (CHUNK_WORLD_SIZE) + z;

			float heightValue = noise.sample(worldX, worldZ); // samples the noise at that world position

			heightmap[x + z * CHUNK_SIZE] = heightValue; // stores the height value in the 1D mapped array
		}
	}

	// upload heightmap of this chunk to GPU buffer here
	// (staging buffer -> device local buffer)

	chunk.generated = true; // this chunk has been generated!!!!
}
