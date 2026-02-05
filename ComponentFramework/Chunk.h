#pragma once
#include "CoreStructs.h"
#include "Component.h"
#include "WorldConstants.h"

class Chunk
{
	Vec2 position; // x,z position in world space. y comes from heightmap
	std::vector<float> heightmap; // heightmap for the chunk
	

public:
	//get chunk width/length
	//static constexpr int GetChunkSize() { return CHUNK_SIZE; }

	Chunk(Vec2 pos) { position = pos; }
	~Chunk();

	const std::vector<float>& GetHeightmap() const { return heightmap; }
	const Vec2& GetPosition() const { return position; }

	void SetHeightmap(std::vector<float>&& hm) { heightmap = std::move(hm); }

	Vec2 getChunkPos() const { return position; }

};

