#include "World.h"

void World::createWorld(const TerrainPreset& preset)
{
	terrainNoise = new TerrainNoise(preset);
	//create terrain mesh here
}

void World::update(Vec3 playerPosition)
{
	// determine which chunks to load/generate based on player position
	//Vec2 centerChunkCoord = {int(std::floor(playerPosition.x / CHUNK_WORLD_SIZE)),
	//						  int(std::floor(playerPosition.z / CHUNK_WORLD_SIZE)) };

	//constexpr int RENDER_DISTANCE = 4; // number of chunks to load in each direction

	//for (int z = -RENDER_DISTANCE; z <= RENDER_DISTANCE; z++) {
	//	for (int x = -RENDER_DISTANCE; x <= RENDER_DISTANCE; x++) {
	//		Vec2 chunkCoord = centerChunkCoord + Vec2(float(x), float(z));
	//		
	//		// logic for loading/generating chunks

	//	}
	//}
}

void World::renderWorld(VkCommandBuffer commandBuffer)
{
	//render generated chunks here
}

int64_t World::hashChunkCoord(const Vec2& coord) const
{
	return (int64_t(coord.x) <<32) | uint32_t(coord.y);
}
