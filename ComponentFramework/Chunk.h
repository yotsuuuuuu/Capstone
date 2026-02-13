#pragma once
#include "CoreStructs.h"
#include "Component.h"
#include "WorldConstants.h"

class Chunk
{
	Vec2 position; // x,z position in world space. y comes from heightmap
	std::vector<float> heightmap; // heightmap for the chunk
	
	// TODO: (andres) turn this into a component

public:

	Chunk() { position = Vec2(); }

	Chunk(Vec2 pos) { position = pos; }

	void SetHeightmap(std::vector<float>&& hm) { heightmap = std::move(hm); }
	void SetPosition(Vec2 pos) { position = pos; }

	const std::vector<float>& GetHeightmap() const { return heightmap; }
	const Vec2& GetPosition() const { return position; }


	Vec2 getChunkPos() const { return position; }

};

