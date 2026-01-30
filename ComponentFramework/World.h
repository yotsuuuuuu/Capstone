#pragma once
#include <unordered_map>
#include "Vector.h"
#include "WorldConstants.h"
#include "CoreStructs.h"
#include "TerrainPreset.h"
#include "TerrainNoise.h"
#include "VulkanRenderer.h"
#include "Chunk.h"

using namespace MATH;

class World {
private:
	Renderer* renderer;
	TerrainNoise* terrainNoise;
	//std::unique_ptr<PipelineInfo> terrainPipeline; // pipeline for rendering terrain chunks
	std::vector<std::unique_ptr<Chunk>> chunks; // vector of chunks
	Matrix4 projectionMatrix;
	//int64_t hashChunkCoord(const Vec2& coord) const;

public:
	World(Renderer* renderer_) : renderer(renderer_), terrainNoise(nullptr) {}
	~World();
	void Initialize(TerrainPreset* t_);
	void Render();
	void Update(Vec3 playerPosition); // either pass it deltatime or player position or both, probably both
};