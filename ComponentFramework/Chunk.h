#pragma once
#include "Vector.h"
#include "CoreStructs.h"
constexpr int CHUNK_SIZE_X = 64;
using namespace MATH;

class Chunk
{
public:

	VkBuffer heightmapBuffer = VK_NULL_HANDLE;
	VkDeviceMemory heightmapBufferMemory = VK_NULL_HANDLE;

	bool generated = false;
	Vec2 chunkCoord;
};

