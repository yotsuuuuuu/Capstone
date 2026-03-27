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


void PhysicsManager::TerrainCollision()
{
	playerCollider->UpdateCapsulePoints();
	bool collisionDetected = false;
	//std::cout << "Player Gravity: " << playerPhysics->HasGravity() << "\n" << " IsGrounded: " << playerPhysics->IsGrounded() << std::endl;
		
    
	for (auto& chunk : *chunksData) {

        // skip culled chunks
        if (chunk.isCulled) continue;

        // broad phase AABB test
		if (!playerCollider->SimpleIntersectingAABB(chunk.aabb)) { continue; }

		playerPhysics->SetHasGravity(true); // just so the player is falling if they are in the air, above an AABB (or whatever)

        Vec2 chunkPos = chunk.chunkPos; 

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
			playerCollider->SetCapA(originalCapA + step * (i+1));
			playerCollider->SetCapB(originalCapB + step * (i+1));

			// narrow phase triangle collision
			MeshCollisionInfo mInfo = playerCollider->IntersectingMesh(chunk.vertices, chunkIndices, chunkPos);

			CheckGroundCollision(mInfo); // check if it's ground collision and set grounded state accordingly
			if (mInfo.isColliding) {
				//std::cout << "triangle collision detected: " << mInfo.triangleIndex
				//	<< " at point: " << mInfo.point.x << ", "
				//	<< mInfo.point.y << ", " << mInfo.point.z << std::endl;


				// collision response
				playerCollider->ApplyCollisionResponse(mInfo);
				playerCollider->UpdateCapsulePoints();
				return;
					


			}
			
		}

		playerCollider->SetCapA(originalCapA); // reset capsule points after checking all steps
		playerCollider->SetCapB(originalCapB);

        
    }
}



void PhysicsManager::Update(const float dt)
{
	// run update on all physics actors
	player->Update(dt);
	
	TerrainCollision();

}

void PhysicsManager::CheckGroundCollision(MeshCollisionInfo mInfo)
{
	if (!mInfo.isColliding) {
		playerPhysics->SetIsGrounded(false);
		playerPhysics->SetHasGravity(true);
		return;
	}

	bool isGround = VMath::dot(mInfo.normal, Vec3(0, 1, 0)) > 0.3f;

	if (isGround) {
		playerPhysics->SetIsGrounded(true);
		// stop vertical velocity on ground
		Vec3 vel = playerPhysics->GetVelocity();
		vel.y = 0.0f;
		playerPhysics->SetVelocity(vel);
		playerPhysics->SetHasGravity(false);
	}
	else {
		// it's a wall - handle separately
		playerPhysics->SetIsGrounded(false);
		playerPhysics->SetHasGravity(true);
	}
}

void PhysicsManager::CheckGroundCollision(CollisionInfo info)
{



}


