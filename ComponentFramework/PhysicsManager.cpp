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
	playerPhysics = player->GetComponent<CPhysics>();
	if (!playerPhysics) return false;

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

            //std::cout << "Broad phase collision with chunk at: " << chunkPos.x << ", " << chunkPos.y << std::endl;

           // quarter steps like mario
			Vec3 playerDirection = playerPhysics->GetVelocity();
			if (VMath::mag(playerDirection) < VERY_SMALL) {
				playerDirection = Vec3(0.0f, -1.0f, 0.0f); // if player is not moving, just check downwards
			}
			Vec3 playerDirNormalized = VMath::normalize(playerDirection);
			Vec3 originalCapA = playerCollider->GetCapA();
			Vec3 originalCapB = playerCollider->GetCapB();

			Vec3 step = playerDirNormalized * 0.25f; // quarter steps
			for (int i = 0; i < 4; i++) {
				//playerCollider->UpdateCapsulePoints(); // update capsule points before each step
				playerCollider->SetCapA(originalCapA + step * (i+1));
				playerCollider->SetCapB(originalCapB + step * (i+1));



				// narrow phase triangle collision
				MeshCollisionInfo mInfo = playerCollider->IntersectingMesh(chunk.vertices, chunkIndices, chunkPos);

				//std::cout << mInfo.isColliding << std::endl;

				if (mInfo.isColliding) {
					std::cout << "triangle collision detected: " << mInfo.triangleIndex
						<< " at point: " << mInfo.point.x << ", "
						<< mInfo.point.y << ", " << mInfo.point.z << std::endl;

					// apply collision response
					playerCollider->ApplyCollisionResponse(mInfo);
					// if only need one collision, break here
					// break;
				}
            
			}

			playerCollider->SetCapA(originalCapA); // reset capsule points after checking all steps
			playerCollider->SetCapB(originalCapB);

        }
    }
}

//void PhysicsManager::TerrainCollision()
//{
//    playerCollider->UpdateCapsulePoints();
//
//    // Get current velocity
//    Vec3 velocity = playerPhysics->GetVelocity();
//    float speed = VMath::mag(velocity);
//
//    if (speed < VERY_SMALL) {
//        // Stationary: just check ground collision
//        CheckGroundCollision();
//        return;
//    }
//
//    // Calculate number of steps based on speed and capsule size
//    int numSteps = std::max(1, static_cast<int>(speed * 2.0f) + 1);
//    numSteps = std::min(numSteps, 8); // Cap at 8 steps
//
//    Vec3 stepVelocity = velocity / static_cast<float>(numSteps);
//    Vec3 originalCapA = playerCollider->GetCapA();
//    Vec3 originalCapB = playerCollider->GetCapB();
//    bool collisionOccurred = false;
//
//    for (int step = 0; step < numSteps; step++) {
//        // Move capsule forward by step
//        Vec3 newCapA = originalCapA + stepVelocity * static_cast<float>(step + 1);
//        Vec3 newCapB = originalCapB + stepVelocity * static_cast<float>(step + 1);
//
//        playerCollider->SetCapA(newCapA);
//        playerCollider->SetCapB(newCapB);
//
//        bool stepCollision = false;
//
//        // Check all chunks for this step
//        for (auto& chunk : *chunksData) {
//            if (!chunk.isInitialized || chunk.isCulled) continue;
//
//            // Broad phase
//            if (!playerCollider->SimpleIntersectingAABB(chunk.aabb)) continue;
//
//            // Narrow phase
//            MeshCollisionInfo mInfo = playerCollider->IntersectingMesh(
//                chunk.vertices,
//                chunkIndices,
//                chunk.chunkPos
//            );
//
//            if (mInfo.isColliding) {
//                // Collision found for this step
//                playerCollider->ApplyCollisionResponse(mInfo);
//                stepCollision = true;
//                collisionOccurred = true;
//                break; // Exit chunk loop after first collision
//            }
//        }
//
//        if (stepCollision) {
//            // Update original position to the position after collision response
//            originalCapA = playerCollider->GetCapA();
//            originalCapB = playerCollider->GetCapB();
//
//            // Adjust remaining steps based on new position
//            // Recalculate velocity after collision
//            velocity = playerPhysics->GetVelocity();
//            speed = VMath::mag(velocity);
//            if (speed < VERY_SMALL) break;
//
//            stepVelocity = velocity / static_cast<float>(numSteps - step - 1);
//        }
//    }
//
//    // Final update
//    playerCollider->SetCapA(originalCapA);
//    playerCollider->SetCapB(originalCapB);
//    playerPhysics->SetPosition(playerCollider->GetCapB() - Vec3(0.0f, 0.0f, 0.0f)); // Adjust based on your capsule setup
//
//    // Ground detection
//    CheckGroundCollision();
//}

void PhysicsManager::CheckGroundCollision()
{
    // create a small ray from capsule bottom downward
    Vec3 capsuleBottom = playerCollider->GetCapB();
    float groundCheckDistance = 0.2f;

    for (auto& chunk : *chunksData) {
        if (!chunk.isInitialized || chunk.isCulled) continue;

        // quick AABB check
        AABB groundCheckAABB;
        groundCheckAABB.min = capsuleBottom - Vec3(0.5f, groundCheckDistance, 0.5f);
        groundCheckAABB.max = capsuleBottom + Vec3(0.5f, 0.1f, 0.5f);

        if (!playerCollider->SimpleIntersectingAABB(chunk.aabb)) continue;

        // check ground triangles
        MeshCollisionInfo mInfo = playerCollider->IntersectingMesh(
            chunk.vertices,
            chunkIndices,
            chunk.chunkPos
        );

        if (mInfo.isColliding) {
            // player is on ground
            playerPhysics->SetIsGrounded(true);

            // snap to ground
            Vec3 correction = mInfo.normal * mInfo.penetrationDepth;
            playerPhysics->SetPosition(playerPhysics->GetPosition() + correction);

            // update capsule
            playerCollider->UpdateCapsulePoints();
            return;
        }
    }

    playerPhysics->SetIsGrounded(false);
}

void PhysicsManager::Update(const float dt)
{
	// run update on all physics actors
	player->Update(dt);
	
	TerrainCollision();

}