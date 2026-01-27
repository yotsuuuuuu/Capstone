#pragma once
#include "CoreStructs.h"
#include "Chunk.h"

class TerrainRenderer
{
public:
	void drawChunk(VkCommandBuffer commandBuffer, 
					const TerrainRenderData& mesh, 
					const Chunk& chunk);
};

