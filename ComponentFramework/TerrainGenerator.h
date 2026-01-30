#pragma once
#include "World.h"
// #include "VulkanRenderer.h" maybe?? it's already incuded in the world


class TerrainGenerator
{
private:
	// Uint32 ticksCount = 0;
	std::unique_ptr<World> world = nullptr;

public:
	TerrainGenerator();
	~TerrainGenerator();

	void run();
	
private:
	void initialize();
	void destroy();

	void update(float dt);
	// void Render();
};

