#include "PhysicsManager.h"


bool PhysicsManager::OnCreate()
{
	for (auto& actor : actorsInScene) {
		auto actorComponent = std::dynamic_pointer_cast<CActor>(actor);
		if (!actorComponent) continue;
		auto collider = actorComponent->GetComponent<CCollider>();
		if (!collider) continue;
		// might have to check what the collider type is 
		// check cillider type, cast as it, and store it in the 
		// collider vector and maybe the inheritence should work??
		// for now just store as base collider and use virtual functions to do the collision check,
		// but if we want to do some specific optimizations for different collider types then we can 
		// store them in separate vectors and do the collision check based on the type of collider.


		collidersInScene.push_back(actor); // maybe direct collider instead(?)
	}

	worldActor = std::dynamic_pointer_cast<CActor>(world);
	if (!worldActor) return false;
	worldComp = worldActor->GetComponent<CWorld>();
	if (!worldComp) return false;
	chunksData = worldComp->GetChunkRenderData();
	indices = worldComp->GetChunkIndices();
	world_size = worldComp->GetWorldSize();
	chunkMap = worldComp->GetChunkMap();

	player = std::dynamic_pointer_cast<CActor>(camera);
	if (!player) return false;
	playerCollider = player->GetComponent<CCapsuleCollider>();
	if (!playerCollider) return false;


	return false;
}

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