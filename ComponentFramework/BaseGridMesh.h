#pragma once
#include "Vector.h"
#include <vector>
using namespace MATH;

struct BaseGridMesh
{
	std::vector<Vec3> basePositions; // only really need x,z initially. y is done by individual chunk heightmap
	std::vector<Vec2> baseUVs;
	std::vector<uint32_t> baseIndices;
	//IndexBuffer* indexBuffer;

};

	BaseGridMesh GenerateMesh(int size);
