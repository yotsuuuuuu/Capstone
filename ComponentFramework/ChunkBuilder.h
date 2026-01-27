#pragma once
#include "TerrainNoise.h"
#include "Chunk.h"

class ChunkBuilder
{
public: 
	void generateChunk(Chunk& chunk, const TerrainNoise& noise);
};

