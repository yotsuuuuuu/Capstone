#include "World.h"
#include "FmodController.h"


void World::Initialize(TerrainPreset* t_)
{
	vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);

	terrainNoise = new TerrainNoise(*t_);
	baseChunkMesh = std::make_unique<BaseGridMesh>(GenerateMesh(CHUNK_SIZE));

	GenerateAllChunks();
}

void World::Initialize(int songIndex)
{
	vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);
	baseChunkMesh = std::make_unique<BaseGridMesh>(GenerateMesh(CHUNK_SIZE));


	TerrainPreset preset;
	std::vector<AudioBands> ab = engineContext.fmodController->AnalyzeAudioOffline(songIndex); // TODO: pass in song num
	preset.CreateFromAudio(ab);
	terrainNoise = new TerrainNoise(preset);
	// WorldActors worldActors = preset.DecideActors();
	
	WORLD_SIZE = preset.pAudio.songLength / 250; // this is a really rough way to determine world size based on song length.

	GenerateAllChunks();

}


void World::OnDelete()
{
	delete terrainNoise;
	baseChunkMesh.reset();
	vkDeviceWaitIdle(vRenderer->getDevice());
	vRenderer->DestroyIndexedMesh(chunkIndexBuffer);
	for (const auto& c : chunkRenderData) {
		vRenderer->DestroyTerrainVertexBuffers(c.vertexBuffer);
	}
}



void World::GenerateAllChunks()
{
	chunkMap.clear();
	chunkRenderData.clear();
	chunkRenderData.shrink_to_fit();

	vRenderer->CreateTerrainIndexBuffer(baseChunkMesh->baseIndices, chunkIndexBuffer);
	// create grid of chunks
	int i = 0;
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int y = 0; y < WORLD_SIZE; y++) {

			// it might be better to keep internal pos and world position as separate var.
			Vec2 chunkWorldPos = Vec2((x * CHUNK_WORLD_SIZE) - WORLD_OFFSET, (y * CHUNK_WORLD_SIZE) - WORLD_OFFSET);
			auto tempChunk = std::make_unique<Chunk>(chunkWorldPos);
			//printf("Chunk number: %d\n", i);
			GenerateChunkHeightmap(tempChunk.get());
			BuildChunkMeshData(tempChunk.get());

			//chunkMap.insert({chunkWorldPos, std::move(tempChunk) });
			i++;
		}
	}
}

void World::GenerateChunkHeightmap(Chunk* chunk)
{
	std::vector<float> heightmap(CHUNK_SIZE*CHUNK_SIZE);
	Vec2 chunkPos = chunk->GetChunkPos();
	float minHeight = std::numeric_limits<float>::max();
	float maxHeight = std::numeric_limits<float>::lowest();

	for (int z = 0; z < CHUNK_SIZE; z++) {
		for (int x = 0; x < CHUNK_SIZE; x++) {

			// get world position
			float worldX = chunkPos.x + float(x);
			float worldZ = chunkPos.y + float(z);
			float heightValue = terrainNoise->sample(worldX, worldZ);

			heightmap[z * CHUNK_SIZE + x] = heightValue;

			if (heightValue < minHeight) minHeight = heightValue;
			if (heightValue > maxHeight) maxHeight = heightValue;
		}
	}
	chunk->SetHeightmap(std::move(heightmap));
	chunk->setMinY(minHeight);
	chunk->setMaxY(maxHeight);
	chunk->SetWorldPos((minHeight + maxHeight) / 2.0f); // set world pos y to the middle of the chunk height range for culling

	if (minHeight < lowestPoint) lowestPoint = minHeight;
	if (maxHeight > highestPoint) highestPoint = maxHeight;
}

void World::BuildChunkMeshData(Chunk* chunk)
{
	std::vector<Vertex> vertices;
	//std::vector<Vertex> collisionVertices;
	vertices.reserve(baseChunkMesh->basePositions.size());

	const auto& heightmap = chunk->GetHeightmap();
	const Vec2& chunkPos = chunk->GetChunkPos();
	const Vec3& chunkWorldPos = chunk->GetWorldPos();

	for (size_t i = 0; i < baseChunkMesh->basePositions.size(); i++) {
		Vertex vertex;
		Vec3 basePos = baseChunkMesh->basePositions[i]; // original position of the OG mesh

		vertex.pos = Vec3( // local space position of vertex in world space, y will be adjusted by heightmap
			basePos.x,
			heightmap[i],
			basePos.z); 

		vertex.texCoord = baseChunkMesh->baseUVs[i];
		vertex.normal = Vec3(0.0f, 1.0f, 0.0f); // temporary normal, will be calculated later
		//vertex.position.print("vertext postion");
		//vertex.uv.print("vertext UV");
		//vertex.normal.print("vertext Normal");
		vertices.push_back(vertex);

		//Vertex collisionVertex;
		//collisionVertex.pos = Vec3( // WORLD space position of vertex, used for collision checking
		//	chunkPos.x + basePos.x,
		//	heightmap[i],
		//	chunkPos.y + basePos.z);
		//collisionVertices.push_back(collisionVertex);

	}

	CalculateNormals(vertices);
	
	// Store the mesh data in the chunkRenderDataMap
	TerrainChunkData renderData;


	renderData.transform.modelMatrix = MMath::translate(Vec3(chunkPos.x,0.0f,chunkPos.y)); 
	renderData.transform.normalMatrix = MMath::transpose(MMath::inverse(renderData.transform.modelMatrix)); 

	// make indices once. store in world. then make vertices and pass the indices
	renderData.vertexBuffer.indexBufferID = chunkIndexBuffer.indexBufferID;
	renderData.vertexBuffer.indexBufferLength = chunkIndexBuffer.indexBufferLength;
	renderData.vertexBuffer.indexBufferMemoryID = chunkIndexBuffer.indexBufferMemoryID;

	vRenderer->CreateTerrainVertexBuffer(vertices, renderData.vertexBuffer);

	renderData.isInitialized = true;
	renderData.aabb.min = Vec3(chunkPos.x, chunk->getMinY(), chunkPos.y);
	renderData.aabb.max = Vec3(chunkPos.x + CHUNK_WORLD_SIZE, chunk->getMaxY(), chunkPos.y + CHUNK_WORLD_SIZE);
	renderData.vertices = std::move(vertices);
	renderData.chunkPos = chunkPos;
	chunkRenderData.push_back(renderData);
	//chunkRenderData[chunkWorldPos] = renderData;

}

void World::CalculateNormals(std::vector<Vertex>& vertices)
{
	for (auto& vertex : vertices) {
		vertex.normal = Vec3(0.0f, 0.0f, 0.0f); // reset normals
	}

	const auto& indices = baseChunkMesh->baseIndices;

	for (size_t i = 0; i < indices.size(); i += 3) {
		uint32_t index0 = indices[i];
		uint32_t index1 = indices[i + 1];
		uint32_t index2 = indices[i + 2];

		Vec3& v0 = vertices[index0].pos;
		Vec3& v1 = vertices[index1].pos;
		Vec3& v2 = vertices[index2].pos;

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
	chunkMap.clear();
	//baseChunkMesh.release();
	chunkRenderData.clear();
}



