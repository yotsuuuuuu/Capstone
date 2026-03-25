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


		collidersInScene.push_back(collider); 
	}

	// grabbing chunks from world
	worldActor = std::dynamic_pointer_cast<CActor>(world);
	if (!worldActor) return false;
	worldComp = worldActor->GetComponent<CWorld>();
	if (!worldComp) return false;
	chunksData = worldComp->GetChunkRenderData();
	chunkIndices = worldComp->GetChunkIndices();
	world_size = worldComp->GetWorldSize();
	chunkMap = worldComp->GetChunkMap();

	player = std::dynamic_pointer_cast<CActor>(camera);
	if (!player) return false;
	playerCollider = player->GetComponent<CCapsuleCollider>();
	if (!playerCollider) return false;


	return false;
}

//void PhysicsManager::TerrainCollision()
//{
//	int mapX = 0;
//	int mapY = 0;
//
//	for (auto& cd : *chunksData) {
//		if (cd.isCulled) continue; // if culled skip collision check
//		bool colliding = playerCollider->SimpleIntersectingAABB(cd.aabb); // rn just player
//		//std::cout << colliding << std::endl;
//		if (colliding) {
//			std::cout << "Colliding with chunk at pos: " << std::endl;
//			// check if colliding with mesh
//			auto chunkIt = chunkMap->find(Vec2(mapX, mapY));
//			if (chunkIt != chunkMap->end()) {
//				std::cout << "Colliding with chunk at pos: " << chunkIt->first.x << ", " << chunkIt->first.y << std::endl;
//				auto& chunk = chunkIt->second;
//				MeshCollisionInfo mInfo = playerCollider->IntersectingMesh(chunk->GetVertices(), indices);
//				if (mInfo.isColliding) {
//					std::cout << "Colliding with triangle index: " << mInfo.triangleIndex << " at point: " << mInfo.point.x << ", " << mInfo.point.y << ", " << mInfo.point.z << std::endl;
//					// 
//					// maybe break here if we only want to know if colliding with the chunk or not, 
//					// but if we want to know how many triangles colliding with then we can keep going and check all chunks in the world.
//				}
//			}
//		}
//
//		mapX++;
//		if (mapX >= world_size) {
//			mapX = 0;
//			mapY++;
//			if (mapY >= world_size) {
//				break;
//			}
//		}
//
//
//	}
//}

void PhysicsManager::TerrainCollision()
{
    for (size_t i = 0; i < chunksData->size(); i++) {
        auto& chunk = (*chunksData)[i];

        // skip culled chunks
        if (chunk.isCulled) continue;

        // broad phase AABB test
        if (playerCollider->SimpleIntersectingAABB(chunk.aabb)) {


            Vec2 chunkPos = chunk.chunkPos; 

            std::cout << "Broad phase collision with chunk at: " << chunkPos.x << ", " << chunkPos.y << std::endl;

           

            // narrow phase triangle collision
			MeshCollisionInfo mInfo = playerCollider->IntersectingMesh(chunk.vertices, chunkIndices);

			std::cout << mInfo.isColliding << std::endl;

            if (mInfo.isColliding) {
                std::cout << "Collision detected! Triangle: " << mInfo.triangleIndex
                    << " at point: " << mInfo.point.x << ", "
                    << mInfo.point.y << ", " << mInfo.point.z << std::endl;

                // apply collision response

                // if only need one collision, break here
                // break;
            }
            
        }
    }
}

void PhysicsManager::Update(const float dt)
{
	// run update on all physics actors
	player->Update(dt);
	
	TerrainCollision();

}