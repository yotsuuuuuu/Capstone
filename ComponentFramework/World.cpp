#include "World.h"
#include "FmodController.h"
#include "AssetManager.h"
#include "CLight.h"
#include "CActor.h"
#include "CMaterial.h"
#include "CMesh.h"
#include <numeric>


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

	lowestPoint = 0;
	highestPoint = 0;
	TerrainPreset preset;
	std::vector<AudioBands> ab = engineContext.fmodController->AnalyzeAudioOffline(songIndex); // TODO: pass in song num
	preset.CreateFromAudio(ab);
	terrainNoise = new TerrainNoise(preset);
	// WorldActors worldActors = preset.DecideActors();
	
	WORLD_SIZE = preset.WORLD_SIZE; 

	worldSeed = preset.pAudio.seed;
	GenerateAllChunks();
	CreateActorSpawns(preset.actorAmount);
	std::cout << "low Point: " << lowestPoint << " High Point: " << highestPoint << std::endl;


	static const Vec3 testColors[] = {
		{1.0f, 0.0f, 0.0f},   // red			//0
		{0.0f, 1.0f, 0.0f},   // green			//1
		{0.0f, 0.0f, 1.0f},   // blue			//2
		{1.0f, 1.0f, 0.0f},   // yellow			//3
		{0.0f, 1.0f, 1.0f},   // cyan			//4
		{1.0f, 0.0f, 1.0f},   // magenta		//5
		{1.0f, 0.5f, 0.0f},   // orange			//6
		{0.5f, 0.0f, 1.0f},   // purple			//7
		{0.0f, 1.0f, 0.5f},   // spring green	//8
		{1.0f, 0.0f, 0.5f},   // rose			//9
	};

	struct colourPairs {
		Vec3 colour1;
		Vec3 colour2;
	};
	std::vector<colourPairs> pairs;

	pairs.push_back(colourPairs{ testColors[2], testColors[3] }); // blue yellow
	pairs.push_back(colourPairs{ testColors[3], testColors[0] }); // yellow red
	pairs.push_back(colourPairs{ testColors[4], testColors[5] }); // cyan magenta
	pairs.push_back(colourPairs{ testColors[6], testColors[2] }); // orange blue
	//pairs.push_back(colourPairs{ testColors[7], testColors[5] }); // purple magenta
	pairs.push_back(colourPairs{ testColors[0], testColors[2] }); // red blue
	pairs.push_back(colourPairs{ testColors[3], testColors[7] }); // yellow purple
	pairs.push_back(colourPairs{ testColors[8], testColors[9] }); // spring green rose
	pairs.push_back(colourPairs{ testColors[9], testColors[4] }); // rose cyan
	pairs.push_back(colourPairs{ testColors[9], testColors[3] }); // rose yellow
	pairs.push_back(colourPairs{ testColors[0], testColors[5] }); // red magenta
	pairs.push_back(colourPairs{ testColors[2], testColors[4] }); // blue cyan

	auto rng = GetChunkRNG(Vec2(worldSeed / 20.0f, worldSeed / 25.0f), worldSeed * 7.0f);
	std::uniform_int_distribution<int> colourIndices(0, pairs.size());
	int colourIndex = colourIndices(rng);

	if (preset.magicNumber % 2 == 0) {
		vRenderer->UpdateTerrainMinColor(pairs[colourIndex].colour1);
		vRenderer->UpdateTerrainMaxColor(pairs[colourIndex].colour2);
	}
	else {
		vRenderer->UpdateTerrainMaxColor(pairs[colourIndex].colour1);
		vRenderer->UpdateTerrainMinColor(pairs[colourIndex].colour2);

	}


}


void World::OnDelete()
{
	delete terrainNoise;
	baseChunkMesh.reset();
	//vkDeviceWaitIdle(vRenderer->getDevice());
	vRenderer->DestroyIndexedMesh(chunkIndexBuffer);
	//engineContext.assetManager->clearActorsInScene();
	actorlocations.clear();

	chunkMap.clear();

	for (const auto& c : chunkRenderData) {
		vRenderer->DestroyTerrainVertexBuffers(c.vertexBuffer);
	}
	chunkRenderData.clear();
	chunkRenderData.shrink_to_fit();
}



void World::CreateActorSpawns(ActorAmount actorAmount_)
{
	// PLAYER SPAWN //
	// center chunk index (0‑based)
	int centerId = (WORLD_SIZE - 1) / 2; // center chunk
	int center = CHUNK_SIZE / 2; // center OF chunk

	auto chunk = chunkMap.find(Vec2(centerId, centerId)); // middle chunk (chunk space)
	float height = chunk->second->GetHeightAtPosition(center, center, CHUNK_WORLD_SIZE);

	std::cout << "World - player chunk" << "height: " << height << " chunkMax: " << chunk->second->getMaxY() << std::endl;
	if (chunk->second->getMaxY() - height > 10) { height = chunk->second->getMaxY(); } // if bigger than 20 unit gap then just set to max height

	Vec2 chunkWorld = chunk->second->GetChunkPos();
	spawnPoint = Vec3(chunkWorld.x, height + 5.0f, chunkWorld.y); // height w buffer


	// RANDOM ACTOR PLACEMENT
	//int actorPerChunk = actorAmount_.totalActors / (WORLD_SIZE * WORLD_SIZE); // average out number of actors to chunks
	int actorPerChunk = 1; // average out number of actors to chunks
	

	const int MAX_ATTEMPTS = actorPerChunk * 500;
	float minDistance = 5.0f;
	float minDistSq = minDistance * minDistance;

	for (const auto& p : chunkMap) {
		const auto& c = p.second;
		std::vector<Vec3> actorsInChunk;
		Vec2 chunkPos = c->GetChunkPos();
		auto rng = GetChunkRNG(chunkPos, worldSeed*7.0f);
		std::uniform_real_distribution<float> xDist(0.0f, CHUNK_WORLD_SIZE);
		std::uniform_real_distribution<float> zDist(0.0f, CHUNK_WORLD_SIZE);
		int attempts = 0;

		while (actorsInChunk.size() < actorPerChunk && attempts < MAX_ATTEMPTS) {
			float localX = xDist(rng);
			float localZ = zDist(rng);
			float worldX = chunkPos.x + localX;
			float worldZ = chunkPos.y + localZ;
			float y = c->GetHeightAtPosition(localX, localZ, CHUNK_SIZE); 

			Vec3 candidate(worldX, y, worldZ);

			bool tooClose = false;

			for (const auto& pos : actorsInChunk) {
				float dx = candidate.x - pos.x;
				float dz = candidate.z - pos.z;
				if (dx * dx + dz * dz < minDistSq) {
					tooClose = true;
					break;
				}
			}

			if (!tooClose) {
				actorsInChunk.push_back(candidate);
				//candidate.print();
			}

			attempts++;
		}
		actorlocations.insert(actorlocations.end(), std::make_move_iterator(actorsInChunk.begin()), std::make_move_iterator(actorsInChunk.end())); // add the chunk actors to the big list of locations
	}

	int trueLights = ((float)actorAmount_.lights / (float)actorAmount_.totalActors) * actorlocations.size();
	int trueRock1 = ((float)actorAmount_.rock1 / (float)actorAmount_.totalActors) * actorlocations.size();
	int trueRock2 = ((float)actorAmount_.rock2 / (float)actorAmount_.totalActors) * actorlocations.size();
	int trueTree1 = ((float)actorAmount_.tree1 / (float)actorAmount_.totalActors) * actorlocations.size();
	int trueTree2 = ((float)actorAmount_.tree2 / (float)actorAmount_.totalActors) * actorlocations.size();

	int trueTotal = trueLights + trueRock1 + trueRock2 + trueTree1 + trueTree2;

	if ((trueTotal) > actorlocations.size()){
		int temp = trueTotal - actorlocations.size();
		trueLights -= temp;
	}
	else if ((trueTotal) < actorlocations.size()) {
		int temp =  actorlocations.size() - trueTotal;
		trueLights += temp;
	}

	engineContext.assetManager->CreateActor("light", trueLights);
	engineContext.assetManager->CreateActor("mushroom", trueRock1);
	engineContext.assetManager->CreateActor("pillar", trueRock2);
	engineContext.assetManager->CreateActor("LowPolyTree1", trueTree1);
	engineContext.assetManager->CreateActor("mushroom", trueTree2);

	static const Vec3 testColors[] = {
		{1.0f, 0.0f, 0.0f},   // red			//0
		{0.0f, 1.0f, 0.0f},   // green			//1
		{0.0f, 0.0f, 1.0f},   // blue			//2
		{1.0f, 1.0f, 0.0f},   // yellow			//3
		{0.0f, 1.0f, 1.0f},   // cyan			//4
		{1.0f, 0.0f, 1.0f},   // magenta		//5
		{1.0f, 0.5f, 0.0f},   // orange			//6
		{0.5f, 0.0f, 1.0f},   // purple			//7
		{0.0f, 1.0f, 0.5f},   // spring green	//8
		{1.0f, 0.0f, 0.5f},   // rose			//9
	};

	auto& actorsInScene = engineContext.assetManager->GetActorsInScene();
	std::vector<size_t> shuffledIndices = GetShuffledIndices(actorsInScene, std::mt19937(worldSeed));

	const int colorCount = sizeof(testColors) / sizeof(testColors[0]);
	int index = 0;

	std::cout <<"World - " << "actors in scene: " << actorsInScene.size() << std::endl;

	size_t idx = 0;
	//for (int i = 0; i < actorCount; ++i) {
	//	size_t actorIndex = shuffledIndices[idx % shuffledIndices.size()];
	//	// spawn actor using actorMap[actorIndex]
	//}

	for (auto& actor : actorsInScene) {

		auto a = std::dynamic_pointer_cast<CActor>(actor);

		auto light = a->GetComponent<CLight>();
		auto transform = a->GetComponent<CTransform>();
		if (!transform) { 
			continue; 
		}

		int actorIndex = shuffledIndices[idx % shuffledIndices.size()];
		transform->SetPosition(actorlocations[actorIndex]);

		if (light) { 
			Vec3 color = testColors[index % colorCount];
			light->UpdateRadius(25.0f);
			light->UpdateAudioId(index % 10);
			light->UpdateBloomScale(0.9f);
			light->UpdateIntensity(2.0f);
			light->UpdateColour(color);
			transform->SetPosition(transform->GetPosition() + Vec3(0.0f, 5.0f, 0.0f));
			light->UpdateLight();
			index++;
		}
		idx++;

	}


}

void World::GenerateAllChunks()
{
	chunkMap.clear();
	chunkRenderData.clear();
	chunkRenderData.shrink_to_fit();

	vRenderer->CreateTerrainIndexBuffer(baseChunkMesh->baseIndices, chunkIndexBuffer);
	// create grid of chunks
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int y = 0; y < WORLD_SIZE; y++) { // create each chunk

			// it might be better to keep internal pos and world position as separate var.
			Vec2 chunkWorldPos = Vec2((x * CHUNK_WORLD_SIZE) - WORLD_OFFSET, (y * CHUNK_WORLD_SIZE) - WORLD_OFFSET);
			auto tempChunk = std::make_unique<Chunk>(chunkWorldPos);
			//printf("Chunk number: %d\n", i);
			GenerateChunkHeightmap(tempChunk.get());
			BuildChunkMeshData(tempChunk.get());

			//chunkMap.insert({chunkWorldPos, std::move(tempChunk) });
			chunkMap.insert({Vec2(x,y), std::move(tempChunk)});
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

	if (minHeight < lowestPoint) lowestPoint = minHeight;
	if (maxHeight > highestPoint) highestPoint = maxHeight;
}

void World::BuildChunkMeshData(Chunk* chunk)
{
	std::vector<Vertex> vertices;
	//std::vector<Vertex> collisionVertices;
	vertices.reserve(baseChunkMesh->basePositions.size());

	const auto& heightmap = chunk->GetHeightmap();
	const Vec2& chunkPos = chunk->GetChunkPos(); // WORLD SPACE

	for (size_t i = 0; i < baseChunkMesh->basePositions.size(); i++) {
		Vertex vertex;
		Vec3 basePos = baseChunkMesh->basePositions[i]; // original position of the OG mesh

		vertex.pos = Vec3( // local space position of vertex , y will be adjusted by heightmap
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

uint32_t World::HashChunkCoord(int x, int y, uint32_t globalSeed)
{
	return x * 73856093 ^ y * 1934966323487 ^ globalSeed;
}

std::mt19937 World::GetChunkRNG(const Vec2& chunkPos, uint32_t globalSeed)
{
	int cx = static_cast<int>(chunkPos.x / CHUNK_SIZE);
	int cy = static_cast<int>(chunkPos.y / CHUNK_SIZE);
	uint32_t seed = HashChunkCoord(cx, cy, globalSeed);
	return std::mt19937(seed);
}

std::vector<size_t> World::GetShuffledIndices(const std::vector<std::shared_ptr<Component>>& actors, std::mt19937 rng)
{
	std::vector<size_t> indices(actors.size());
	std::iota(indices.begin(), indices.end(), 0);
	std::shuffle(indices.begin(), indices.end(), rng);
	return indices;

}

World::~World()
{
	chunkMap.clear();
	baseChunkMesh.release();
	chunkRenderData.clear();
}



