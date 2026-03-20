#include "PhysicsManager.h"

void PhysicsManager::TerrainCollision()
{
	int mapX = 0;
	int mapY = 0;

	for (auto& cd : *chunksData) {
		if (cd.isCulled) continue; // if culled skip collision check
		CollisionInfo info = playerCollider->IntersectingAABB(cd.aabb);
		if (info.isColliding) {
			//std::cout << "Colliding with chunk at pos: " << std::endl;
			// check if colliding with mesh
			auto chunkIt = chunkMap->find(Vec2(mapX, mapY));
			if (chunkIt != chunkMap->end()) {

				auto& chunk = chunkIt->second;
				MeshCollisionInfo mInfo = playerCollider->IntersectingMesh(chunk->GetVertices(), indices);
				if (mInfo.isColliding) {
					//std::cout << "Colliding with chunk mesh at pos: " << aabb.chunkPos.x << ", " << aabb.chunkPos.y << std::endl;
					// 
					// maybe break here if we only want to know if colliding with the chunk or not, 
					// but if we want to know how many triangles colliding with then we can keep going and check all chunks in the world.
				}
			}
		}

		mapX++;
		if (mapX >= world_size) {
			mapX = 0;
			mapY++;
			if (mapY >= world_size) {
				break;
			}
		}


	}
}