#include "CWorld.h"
#include "VulkanRenderer.h"

CWorld::~CWorld()
{
	if (C_World) {

		delete C_World;
		C_World = nullptr;
	}
}

bool CWorld::OnCreate() {
	if (isCreated)
		return true;

	isCreated = true;
	return true;
}

void CWorld::OnDestroy() {

	C_World->OnDelete();
}

void CWorld::InitializeWorld(TerrainPreset* t_)
{
	C_World->Initialize(t_);
}

void CWorld::InitializeWorld(int songIndex)
{
	C_World->Initialize(songIndex);
	float min = C_World->GetLowestPoint();
	float max = C_World->GetHighestPoint() + C_World->GetLowestPoint() / 2.0f;
	dynamic_cast<VulkanRenderer*>(cntx.renderer)->UpdateTerrainMaxMinHieght(min, max);
}

