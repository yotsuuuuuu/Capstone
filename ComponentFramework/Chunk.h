#pragma once
#include "CoreStructs.h"
#include "Component.h"
#include "WorldConstants.h"

class Chunk
{
	Vec2 position; // x,z position in WORLD space. y comes from heightmap
	std::vector<float> heightmap; // heightmap for the chunk

	// with min and max and the chunk size we can calculate a bounding box for the chunk.
	// can be used for culling and collision detection.
	float minY;
	float maxY;

	std::vector<Vertex> vertices;
	//std::vector<uint32_t> indices;

public:

	Chunk() { position = Vec2(); }

	Chunk(Vec2 pos) { position = pos; }

	void SetHeightmap(std::vector<float>&& hm) { heightmap = std::move(hm); }
	void SetPosition(Vec2 pos) { position = pos; }

	const std::vector<float>& GetHeightmap() const { return heightmap; }
	Vec2 GetChunkPos() const { return position; }

	void setMinY(float min_) { minY = min_; }
	void setMaxY(float max_) { maxY = max_; }

	float getMinY() const { return minY; }
	float getMaxY() const { return maxY; }

	const float GetHeightAtPosition(int x, int z, int chunkWorldSize) const { return heightmap[z * chunkWorldSize + x]; }
	std::vector<Vertex>& GetVertices() { return vertices; }

	//void SetCulled(bool culled_) { culled = culled_; }
	//bool IsCulled() const { return culled; }

};

