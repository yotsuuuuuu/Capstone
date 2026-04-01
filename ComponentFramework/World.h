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
	std::unordered_map<Vec2, std::unique_ptr<Chunk>> chunkMap; // map chunk positions to their render data
	std::vector<TerrainChunkData> chunkRenderData;
	IndexedVertexBuffer chunkIndexBuffer;

	std::unordered_map<int, std::vector<Vec3>> actorChunkLocations;

	int WORLD_SIZE = 16; // number of chunks along one axis (world is WORLD_SIZE x WORLD_SIZE chunks) just two for now
	float WORLD_OFFSET = (CHUNK_SIZE * WORLD_SIZE) / 2.0f;

	float lowestPoint = std::numeric_limits<float>::max(); 
	float highestPoint = std::numeric_limits<float>::min();

	//player
	//CActor* player;
	Vec3 spawnPoint;

	//texture
	Sampler2D terrainTexture;

	// TODO: (andres) LOD/ render distance
	
public:
	
	World(EngineContext& engineContext_) : engineContext(engineContext_), terrainNoise(nullptr) {}
	~World();

	void Initialize(TerrainPreset* t_);
	void Initialize(int songIndex);
	void OnDelete();

	int GetWorldSize() const { return WORLD_SIZE; }
	float GetHighestPoint() const { return highestPoint; }
	float GetLowestPoint() const { return lowestPoint; }

	std::vector<uint32_t> GetChunkIndices() const { return baseChunkMesh->baseIndices; }

	std::unordered_map<Vec2, std::unique_ptr<Chunk> >* GetChunkMap() { return &chunkMap; }
	std::vector<TerrainChunkData>* GetChunkRenderData() { return &chunkRenderData; }
	
	void CreateActorSpawns(ActorAmount actorAmount_);

	Vec3 GetPlayerSpawn() { return spawnPoint; }

private:
	void GenerateAllChunks();
	void GenerateChunkHeightmap(Chunk* chunk);
	void BuildChunkMeshData(Chunk* chunk);
	void CalculateNormals(std::vector<Vertex>& vertices);

};