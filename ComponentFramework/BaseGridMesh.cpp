#include "BaseGridMesh.h"

// size is number of vertices along one edge (size x size) 
BaseGridMesh GenerateMesh(int size)
{
	BaseGridMesh mesh;
	for (int z = 0; z < size; z++) {
		for (int x = 0; x < size; x++) {
			mesh.basePositions.push_back(Vec3(float(x), 0.0f, float(z))); // y will be set later by heightmap
			mesh.baseUVs.push_back(Vec2(float(x) / float(size - 1), float(z) / float(size - 1))); // uv from 0 to 1 across the grid
		}
	}

	for (int z = 0; z < size - 1; z++) {
		for (int x = 0; x < size - 1; x++) {
			uint32_t topLeft = z * size + x;
			uint32_t topRight = topLeft + 1;
			uint32_t bottomLeft = (z + 1) * size + x;
			uint32_t bottomRight = bottomLeft + 1;

			// first triangle
			mesh.baseIndices.push_back(topLeft);
			mesh.baseIndices.push_back(bottomLeft);
			mesh.baseIndices.push_back(topRight);

			// second triangle
			mesh.baseIndices.push_back(topRight);
			mesh.baseIndices.push_back(bottomLeft);
			mesh.baseIndices.push_back(bottomRight);
		}
	}

	return mesh;
}
