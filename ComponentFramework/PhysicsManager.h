#pragma once
#include "World.h"
#include "CActor.h"
#include "CCamera.h"
#include "CWorld.h"
#include "CInput.h"

#include "CPhysics.h"
#include "CoreStructs.h"
//#include "Component.h"
#include "CCapsuleCollider.h"

#include <memory>
#include <Vector.h>

using namespace MATH;

//class CWorld;

class PhysicsManager
{
private:

	Ref<Component> world;
	Ref<Component> camera;

	Ref<CActor> worldActor;// = std::dynamic_pointer_cast<CActor>(world);
	Ref<CWorld> worldComp;// = std::dynamic_pointer_cast<CWorld>(worldActor);

	std::vector<TerrainChunkData>* chunksData;// = worldComp->GetChunkRenderData();
	std::vector<uint32_t> chunkIndices;// = worldComp->GetChunkIndices();
	int world_size;// = worldComp->GetWorldSize();

	std::unordered_map<Vec2, std::unique_ptr<Chunk>>* chunkMap;// = worldComp->GetChunkMap();

	Ref<CActor> player;// = std::dynamic_pointer_cast<CActor>(camera);
	Ref<CCapsuleCollider> playerCollider;// = player->GetComponent<CCapsuleCollider>();
	Ref<CPhysics> playerPhysics;// = player->GetComponent<CPhysics>();

	std::vector<Ref<Component>> actorsInScene;
	std::vector<Ref<CCollider>> collidersInScene;

public:

	void Set(std::vector<Ref<Component>> sceneActors, Ref<Component> cam, Ref<Component> world_) { actorsInScene = sceneActors; camera = cam; world = world_; }
	bool OnCreate();

	void TerrainCollision();

	void Update(const float dt);

private:
	void CheckGroundCollision();
};

