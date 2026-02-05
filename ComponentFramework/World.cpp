#include "World.h"


void World::Initialize(TerrainPreset* t_)
{
	terrainNoise = new TerrainNoise(*t_);
	baseChunkMesh = std::make_unique<BaseGridMesh>(GenerateMesh(CHUNK_SIZE));

	GenerateAllChunks();
}

void World::RenderWorld()
{
}




void World::GenerateAllChunks()
{
	chunks.clear();
	chunkRenderData.clear();

	// create grid of chunks
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int y = 0; y < WORLD_SIZE; y++) {

			Vec2 chunkWorldPos = Vec2(x * CHUNK_WORLD_SIZE, y * CHUNK_WORLD_SIZE);
			auto tempChunk = std::make_unique<Chunk>(chunkWorldPos);

			GenerateChunkHeightmap(tempChunk.get());
			BuildChunkMeshData(tempChunk.get());
			CreateChunkVulkanBuffers(tempChunk.get());

			chunks.push_back(std::move(tempChunk));
		}
	}
}

void World::GenerateChunkHeightmap(Chunk* chunk)
{
	std::vector<float> heightmap(CHUNK_SIZE*CHUNK_SIZE);
	Vec2 chunkPos = chunk->getChunkPos();

	for (int z = 0; z < CHUNK_SIZE; z++) {
		for (int x = 0; x < CHUNK_SIZE; x++) {

			// get world position
			float worldX = chunkPos.x + float(x);
			float worldZ = chunkPos.y + float(z);

			heightmap[z * CHUNK_SIZE + x] = terrainNoise->sample(worldX, worldZ);
		}
	}
	chunk->SetHeightmap(std::move(heightmap));
}

void World::BuildChunkMeshData(Chunk* chunk)
{
	std::vector<TerrainVertex> vertices;
	vertices.reserve(baseChunkMesh->basePositions.size());

	const auto& heightmap = chunk->GetHeightmap();
	const Vec2& chunkPos = chunk->getChunkPos();

	for (size_t i = 0; i < baseChunkMesh->basePositions.size(); i++) {
		TerrainVertex vertex;

		Vec3 basePos = baseChunkMesh->basePositions[i]; // original position of the OG mesh
		//vertex.position
		// adjust x and z position based on chunk position in world
		int x = static_cast<int>(basePos.x);
		int z = static_cast<int>(basePos.z);
		basePos.x += chunkPos.x; // move to chunk world position
		basePos.z += chunkPos.y;

		// adjust y position based on heightmap
		float height = heightmap[z * CHUNK_SIZE + x];
		basePos.y = height;

		vertex.uv = baseChunkMesh->baseUVs[i];
		vertex.normal = Vec3(0.0f, 1.0f, 0.0f); // temporary normal, will be calculated later
		vertex.position = basePos; // base pos at this point is the modified world position
	}

	CalculateNormals(vertices, baseChunkMesh->baseIndices);
	
	// Store the mesh data in the chunkRenderDataMap
	TerrainChunkData renderData;

	// initially identity matrix since positions are in world space already
	renderData.transform.modelMatrix = MMath::translate(Vec3(0.0f, 0.0f, 0.0f)); 
	renderData.transform.normalMatrix = MMath::transpose(MMath::inverse(renderData.transform.modelMatrix)); // should be identity too

	// create vulkan buffers
	// renderer->CreateVertexBuffer(renderData.vertexBuffer, vertices);
	// renderer->CreateIndexBuffer(renderData.vertexBuffer, baseChunkMesh->baseIndices);

	renderData.vertexBuffer.vertBufferLength = vertices.size() * sizeof(TerrainVertex);
	renderData.vertexBuffer.indexBufferLength = baseChunkMesh->baseIndices.size() * sizeof(uint32_t);

	renderData.isInitialized = true;

	chunkRenderData[chunkPos] = renderData;

}

void World::CreateChunkVulkanBuffers(Chunk* chunk)
{

	// call vulkan renderer to create buffers for this chunk

	// TerrainChunkData& renderData = chunkRenderData[chunk->getChunkPos()];
	// renderer->CreateVertexBuffer(renderData.vertexBuffer, vertices);

}

void World::CalculateNormals(std::vector<TerrainVertex>& vertices, const std::vector<uint32_t>& indices)
{
	for (auto& vertex : vertices) {
		vertex.normal = Vec3(0.0f, 0.0f, 0.0f); // reset normals
	}

	for (size_t i = 0; i < indices.size(); i += 3) {
		uint32_t index0 = indices[i];
		uint32_t index1 = indices[i + 1];
		uint32_t index2 = indices[i + 2];

		Vec3& v0 = vertices[index0].position;
		Vec3& v1 = vertices[index1].position;
		Vec3& v2 = vertices[index2].position;

		Vec3 edge1 = v1 - v0;
		Vec3 edge2 = v2 - v0;
		Vec3 faceNormal = VMath::cross(edge1, edge2);

		vertices[index0].normal += faceNormal;
		vertices[index1].normal += faceNormal;
		vertices[index2].normal += faceNormal;
	}

	for (auto& vertex : vertices) {
		if (VMath::mag(vertex.normal) > 0.0f){
			vertex.normal = VMath::normalize(vertex.normal);
		}
		else {
		vertex.normal = Vec3 (0.0f,1.0f,0.0f);
		}
	}

}



World::~World()
{
	chunks.clear();
	delete terrainNoise;
}



