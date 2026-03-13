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
//#include "FmodController.h" // onyl needed for struct. should be move to core stuct methinks
#include "EngineContext.h"

using namespace MATH;

class FmodController; // forward declaration

class World {
private:
	Renderer* renderer;
	VulkanRenderer* vRenderer;
	EngineContext engineContext;
	TerrainNoise* terrainNoise;

	// chunks
	std::unique_ptr<BaseGridMesh> baseChunkMesh; // base mesh for chunks
	std::vector<std::unique_ptr<Chunk>> chunks; 
	std::unordered_map<Vec3, TerrainChunkData> chunkRenderData; // map chunk positions to their render data
	IndexedVertexBuffer chunkIndexBuffer;
	// vulkan
	PipelineInfo worldPipeline;
	DescriptorSetInfo worldDescriptorSet;

	int WORLD_SIZE = 16; // number of chunks along one axis (world is WORLD_SIZE x WORLD_SIZE chunks) just two for now
	float WORLD_OFFSET = (CHUNK_SIZE * WORLD_SIZE) / 2.0f;

	float lowestPoint = 0.0f; // for lowring the entinre mesh if its way above y=0.


	//player
	//CActor* player;

	//texture
	Sampler2D terrainTexture;

	// TODO: (andres) collision meshes
	// TODO: (andres) LOD/ render distance
	
public:
	
	World(EngineContext& engineContext_) : engineContext(engineContext_), terrainNoise(nullptr) {}
	~World();

	void Initialize(TerrainPreset* t_);
	void Initialize(int songIndex);
	void OnDelete();

	PipelineInfo const GetPipeline() { return worldPipeline; }
	DescriptorSetInfo const GetDescriptorSetInfo() { return worldDescriptorSet; }
	std::unordered_map<Vec3, TerrainChunkData>* GetChunkRenderData() { return &chunkRenderData; }
	std::vector<std::unique_ptr<Chunk>>* GetChunks() { return &chunks; }
	// get culled chunk data


private:
	void GenerateAllChunks();
	void GenerateChunkHeightmap(Chunk* chunk);
	void BuildChunkMeshData(Chunk* chunk);
	void CalculateNormals(std::vector<Vertex>& vertices);
};