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
	VulkanRenderer* vRenderer;
	TerrainNoise* terrainNoise;

	// chunks
	std::unique_ptr<BaseGridMesh> baseChunkMesh; // base mesh for chunks
	std::vector<std::unique_ptr<Chunk>> chunks; 
	std::unordered_map<Vec2, TerrainChunkData> chunkRenderData; // map chunk positions to their render data

	// vulkan
	PipelineInfo worldPipeline;
	DescriptorSetInfo worldDescriptorSet;



	//player
	//CActor* player;

	//texture
	Sampler2D terrainTexture;

	// TODO: (andres) collision meshes
	// TODO: (andres) LOD/ render distance
	// TODO: (andres) turn this into a component
	// TODO: (andres) tweak the pipeline more and maybe use compute shaders for heightmap generation and normal calculation. avoids reusing the mesh

public:
	
	World(Renderer* renderer_) : renderer(renderer_), terrainNoise(nullptr) {}
	~World();

	void Initialize(TerrainPreset* t_);
	void RenderWorld();
	void OnDelete();

	PipelineInfo const GetPipeline() { return worldPipeline; }
	DescriptorSetInfo const GetDescriptorSetInfo() { return worldDescriptorSet; }
	std::unordered_map<Vec2, TerrainChunkData> GetChunkRenderData() { return chunkRenderData; }

private:
	void GenerateAllChunks();
	void GenerateChunkHeightmap(Chunk* chunk);
	void BuildChunkMeshData(Chunk* chunk);
	void CalculateNormals(std::vector<Vertex>& vertices);

	void CreateWorldPipeline(std::vector<BufferMemory> cameraUBO_, std::vector<BufferMemory> lightsUBO_);
	void CreateWorldDescriptorSet(std::vector<BufferMemory> cameraUBO, std::vector<BufferMemory> lightsUBO);
};