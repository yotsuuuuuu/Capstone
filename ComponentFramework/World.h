#pragma once
#include <unordered_map>
#include "Vector.h"
#include "WorldConstants.h"
#include "CoreStructs.h"
#include "TerrainPreset.h"
#include "TerrainNoise.h"
#include "VulkanRenderer.h"
#include "Chunk.h"
#include "BaseGridMesh.h"

using namespace MATH;

class World {
private:
	Renderer* renderer;
	TerrainNoise* terrainNoise;
	std::unique_ptr<BaseGridMesh> baseChunkMesh; // base mesh for chunks

	// vector of chunks
	std::vector<std::unique_ptr<Chunk>> chunks; 

	std::unordered_map<Vec2, TerrainChunkData> chunkRenderData; // map chunk positions to their render data

public:

	World(Renderer* renderer_) : renderer(renderer_), terrainNoise(nullptr) {}
	~World();

	void Initialize(TerrainPreset* t_);
	void RenderWorld();

private:
	void GenerateAllChunks();
	void GenerateChunkHeightmap(Chunk* chunk);
	void BuildChunkMeshData(Chunk* chunk);
	void CreateChunkVulkanBuffers(Chunk* chunk);

	void CalculateNormals(std::vector<TerrainVertex>& vertices, const std::vector<uint32_t>& indices);
	//void Update(Vec3 playerPosition); // either pass it deltatime or player position or both, probably both
};