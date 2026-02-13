#include "CWorld.h"

bool CWorld::OnCreate() {
	if (isCreated)
		return true;
	// TODO: needs adjustment
	//C_World->Initialize(&worldConfig,);

	isCreated = true;
	return true;
}

void CWorld::OnDestroy() {

	C_World->OnDelete();
}

void CWorld::InitializeWorld(TerrainPreset* t_, std::vector<BufferMemory> cameraUBO_, std::vector<BufferMemory> lightsUBO_)
{
	C_World->Initialize(t_, cameraUBO_, lightsUBO_);
}

