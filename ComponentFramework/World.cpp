#include "World.h"


void World::Initialize(TerrainPreset* t_)
{
	vRenderer = dynamic_cast<VulkanRenderer*>(renderer);

	terrainNoise = new TerrainNoise(*t_);
	baseChunkMesh = std::make_unique<BaseGridMesh>(GenerateMesh(CHUNK_SIZE));

	// TODO: change texture to something real
	//terrainTexture = vRenderer->Create2DTextureImage("./textures/rock.png");

	//CreateWorldPipeline(cameraUBO_, lightsUBO_);

	GenerateAllChunks();
}

void World::RenderWorld()
{


	for (const auto& chunk : chunks) {
		const Vec2& position = chunk->getChunkPos();

		auto it = chunkRenderData.find(position);
		if (it == chunkRenderData.end()) {
			continue; // no render data found for this chunk
		}

		TerrainChunkData& renderData = it->second;
		if (!renderData.isInitialized) {
			continue; // render data not initialized
		}
		vRenderer->RenderTerrainChunk(renderData.vertexBuffer, renderData.transform, worldPipeline, worldDescriptorSet);
	}
}

void World::OnDelete()
{
	//vRenderer->DestroySampler2D(terrainTexture);
	//vRenderer->DestroyPipeline(worldPipeline);
	for (const auto& pair : chunkRenderData) {
		vRenderer->DestroyIndexedMesh(pair.second.vertexBuffer);
	}

	//vRenderer->DestroyDescriptorSet(worldDescriptorSet);


}


void World::GenerateAllChunks()
{
	chunks.clear();
	chunkRenderData.clear();

	// create grid of chunks
	int i = 0;
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int y = 0; y < WORLD_SIZE; y++) {

			Vec2 chunkWorldPos = Vec2(x * CHUNK_WORLD_SIZE, y * CHUNK_WORLD_SIZE);
			auto tempChunk = std::make_unique<Chunk>(chunkWorldPos);
			//printf("Chunk number: %d\n", i);
			GenerateChunkHeightmap(tempChunk.get());
			BuildChunkMeshData(tempChunk.get());

			chunks.push_back(std::move(tempChunk));
			i++;
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
			//printf("heightValue: %f X:%f, Y %f \t", heightmap[z * CHUNK_SIZE + x],worldX,worldZ);
		}
	}
	chunk->SetHeightmap(std::move(heightmap));
}

void World::BuildChunkMeshData(Chunk* chunk)
{
	std::vector<Vertex> vertices;
	vertices.reserve(baseChunkMesh->basePositions.size());

	const auto& heightmap = chunk->GetHeightmap();
	const Vec2& chunkPos = chunk->getChunkPos();

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
	}

	CalculateNormals(vertices);
	
	// Store the mesh data in the chunkRenderDataMap
	TerrainChunkData renderData;


	renderData.transform.modelMatrix = MMath::translate(Vec3(chunkPos.x,0.0f,chunkPos.y)); 
	renderData.transform.normalMatrix = MMath::transpose(MMath::inverse(renderData.transform.modelMatrix)); 

	vRenderer->CreateTerrainBuffers(vertices, baseChunkMesh->baseIndices, renderData.vertexBuffer);

	renderData.isInitialized = true;
	chunkRenderData[chunkPos] = renderData;

}

void World::CreateWorldPipeline(std::vector<BufferMemory> cameraUBO_, std::vector<BufferMemory> lightsUBO_)
{
	CreateWorldDescriptorSet(cameraUBO_, lightsUBO_);

	worldPipeline = vRenderer->CreateTerrainPipeline(worldDescriptorSet.descriptorSetLayout);

}

void World::CreateWorldDescriptorSet(std::vector<BufferMemory> cameraUBO, std::vector<BufferMemory> lightsUBO)
{
	// this will probly change due to write changes
	std::vector<SingleDescriptorSetLayoutInfo> terrainLayoutInfo;

	SingleDescriptorSetLayoutInfo cameraBinding{};
	cameraBinding.binding = 0;
	cameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraBinding.descriptorCount = 1;
	cameraBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	terrainLayoutInfo.push_back(cameraBinding);


	SingleDescriptorSetLayoutInfo lightsBinding{};
	lightsBinding.binding = 1;
	lightsBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightsBinding.descriptorCount = 1;
	lightsBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	terrainLayoutInfo.push_back(lightsBinding);

	SingleDescriptorSetLayoutInfo textureBinding{};
	textureBinding.binding = 2;
	textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureBinding.descriptorCount = 1;
	textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	terrainLayoutInfo.push_back(textureBinding);


	VkDescriptorSetLayout terrainLayout = vRenderer->CreateDescriptorSetLayout(terrainLayoutInfo);


	VkDescriptorPool terrainPool = vRenderer->CreateDescriptorPool(terrainLayoutInfo, 1);


	std::vector<VkDescriptorSet> terrainSets = vRenderer->AllocateDescriptorSets(terrainPool, terrainLayout);

	std::vector<DescriptorWriteInfo> terrainWriteInfo;

	DescriptorWriteInfo cameraWrite{};
	cameraWrite.binding = 0;
	cameraWrite.type = DescriptorWriteInfo::Destype::UBO;
	cameraWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraWrite.descriptorCount = 1;
	cameraWrite.bufferMem = cameraUBO; 
	terrainWriteInfo.push_back(cameraWrite);

	DescriptorWriteInfo lightsWrite{};
	lightsWrite.binding = 1;
	lightsWrite.type = DescriptorWriteInfo::Destype::UBO;
	lightsWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightsWrite.descriptorCount = 1;
	lightsWrite.bufferMem = lightsUBO;  
	terrainWriteInfo.push_back(lightsWrite);

	DescriptorWriteInfo textureWrite{};
	textureWrite.binding = 2;
	textureWrite.type = DescriptorWriteInfo::Destype::TEXTURE;
	textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureWrite.descriptorCount = 1;
	textureWrite.samplers = { terrainTexture };
	terrainWriteInfo.push_back(textureWrite);

	vRenderer->WriteDescriptorSets(terrainSets, terrainWriteInfo);


	worldDescriptorSet.descriptorSetLayout = terrainLayout;
	worldDescriptorSet.descriptorPool = terrainPool;
	worldDescriptorSet.descriptorSet = terrainSets;


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
	chunks.clear();
	delete terrainNoise;
}



