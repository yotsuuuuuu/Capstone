#include "World.h"


void World::Initialize(TerrainPreset* t_)
{
	terrainNoise = new TerrainNoise(*t_);
	Worl
}

World::~World()
{
	chunks.clear();
	// delete pipeline if created
	delete terrainNoise;
}



