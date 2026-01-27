#pragma once
#include <unordered_map>
#include "Vector.h"
#include "WorldConstants.h"
#include "CoreStructs.h"
#include "TerrainPreset.h"
#include "TerrainNoise.h"
#include "TerrainRenderer.h"
#include "ChunkBuilder.h"
#include "Chunk.h"

using namespace MATH;

class World {
public:
	void createWorld(const TerrainPreset& preset); // creates a world based on the terrain preset
	void update(Vec3 playerPosition); // updates the world based on player position
	void renderWorld(VkCommandBuffer commandBuffer); // renders the world. must figure out how this happens and how/if it needs a ref to the renderer

private:
	int64_t hashChunkCoord(const Vec2& coord) const; // hashes chunk coordinates for storage in unordered map

	TerrainNoise* terrainNoise = nullptr; // pointer to terrain noise generator
	ChunkBuilder chunkBuilder; // chunk builder instance

	TerrainRenderer terrainRenderer; // terrain renderer instance
	TerrainRenderData renderData; // terrain mesh data

	std::unordered_map<int64_t, Chunk> chunks; // map of chunks using hashed coordinates
};