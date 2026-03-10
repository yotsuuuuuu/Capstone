#pragma once
#include "CoreStructs.h"
#include "Component.h"
#include "WorldConstants.h"

class Chunk
{
	Vec2 position; // x,z position in world space. y comes from heightmap
	std::vector<float> heightmap; // heightmap for the chunk
	Vec3 worldPos;
	// with min and max and the chunk size we can calculate a bounding box for the chunk.
	// can be used for culling and collision detection.
	float min;
	float max;
	
	// for culling just figure out the centre world position

public:

	Chunk() { position = Vec2(); }

	Chunk(Vec2 pos) { position = pos; }

	void SetHeightmap(std::vector<float>&& hm) { heightmap = std::move(hm); }
	void SetPosition(Vec2 pos) { position = pos; }
	void SetWorldPos(float y) { worldPos = Vec3(position.x, y, position.y); }

	const std::vector<float>& GetHeightmap() const { return heightmap; }
	const Vec2& GetPosition() const { return position; }
	Vec3 GetWorldPos() const { return worldPos; }
	Vec2 getChunkPos() const { return position; }

	void setMin(float min_) { min = min_; }
	void setMax(float max_) { max = max_; }

	float getMin() const { return min; }
	float getMax() const { return max; }


};

