#include "CWorld.h"

bool CWorld::OnCreate()
{
    return true;
}

void CWorld::OnDestroy()
{
}

void CWorld::Update(const float dt)
{
}

void CWorld::OnIntializeWorld(TerrainPreset* t_, std::vector<BufferMemory> cameraUBO_, std::vector<BufferMemory> lightsUBO_)
{
	world = new World(renderer);
	world->Initialize(t_, cameraUBO_, lightsUBO_);

}
