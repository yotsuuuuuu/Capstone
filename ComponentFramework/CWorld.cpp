#include "CWorld.h"

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
}

