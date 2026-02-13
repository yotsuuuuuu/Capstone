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