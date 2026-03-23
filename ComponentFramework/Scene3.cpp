#include <glew.h>
#include <iostream>
#include "Debug.h"
#include "Scene3.h"
#include <MMath.h>
#include "Debug.h"
#include "CActor.h"
#include "CShader.h"
#include "CMesh.h"
#include "CMaterial.h"
#include "CTransform.h"
#include "CCapsuleCollider.h"
#include "CWorld.h"
#include "CInput.h"
#include "CCamera.h"
#include "CPhysics.h"
#include "CGlobalLight.h"
#include "VulkanRenderer.h"
#include "SYS_Light.h"
#include "OpenGLRenderer.h"
#include "AssetManager.h"
#include "FmodController.h"

Scene3::Scene3(EngineContext& context_): 
	Scene(context_) {
	Debug::Info("Created Scene3: ", __FILE__, __LINE__);
}

Scene3::~Scene3() {
}

bool Scene3::OnCreate() {
	
	switch (engineContext.renderer->getRendererType()){
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);		
		
		auto lights = engineContext.assetManager->CreateActor("lightMagenta", 200);
		auto LightMat = engineContext.assetManager->GetMat("SimpleLightMat");
		auto LightMesh = engineContext.assetManager->GetMesh("IcoMesh");

		if (!LightMat->OnCreate()) {
			Debug::Warning("LightMat Failed ", __FILE__, __LINE__);
			return false;
		}
		if (!LightMesh->OnCreate()) {
			Debug::Warning("LightMesh Failed ", __FILE__, __LINE__);
			return false;
		}

		engineContext.assetManager->CreateActor("mario",1);

		actorsInScene = engineContext.assetManager->GetActorsInScene();

		// Spacing = diameter (radius 1 = diameter 2) so lights dont overlap
		const float spacing = 20.0f;
		int gridSize = static_cast<int>(std::ceil(std::sqrt(lights.size())));
		
		static const Vec3 testColors[] = {
			{1.0f, 0.0f, 0.0f},   // red
			{0.0f, 1.0f, 0.0f},   // green
			{0.0f, 0.0f, 1.0f},   // blue
			{1.0f, 1.0f, 0.0f},   // yellow
			{0.0f, 1.0f, 1.0f},   // cyan
			{1.0f, 0.0f, 1.0f},   // magenta
			{1.0f, 0.5f, 0.0f},   // orange
			{0.5f, 0.0f, 1.0f},   // purple
			{0.0f, 1.0f, 0.5f},   // spring green
			{1.0f, 0.0f, 0.5f},   // rose
		};
		const int colorCount = sizeof(testColors) / sizeof(testColors[0]);

		int index = 0;
		for (auto& actor : lights)
		{
			auto act = std::dynamic_pointer_cast<CActor>(actor);
			auto light = act->GetComponent<CLight>();
			auto transform = act->GetComponent<CTransform>();

			if (!light || !transform)
				continue;

			// Calculate grid position on XZ plane
			int row = index / gridSize;
			int col = index % gridSize;

			float offsetX = (gridSize - 1) * spacing * 0.5f;
			float offsetZ = (gridSize - 1) * spacing * 0.5f;

			float x = (col * spacing) - offsetX;
			float z = (row * spacing) - offsetZ;

			transform->SetPosition(Vec3(x, 0.5f, z));

			// Cycle through distinct colors
			Vec3 color = testColors[index % colorCount];
			light->UpdateRadius(10.0f);
			light->UpdateBloomScale(1.0f);
			light->UpdateIntensity(1.00f);
			light->UpdateColour(color);
			light->UpdateLight();

			index++;
		}
		//step 1.3 Materials

		 std::vector<std::string> filepaths = { "./textures/rock.png" };

		Ref<CMaterial> mat3 = std::make_shared<CMaterial>(nullptr, engineContext.renderer, filepaths, engineContext.assetManager->GetShader("main"));
		mat3->OnCreate();

		// step 2 create actors
		


		Ref<CActor> WorldActor = std::make_shared<CActor>(nullptr);
		auto wC = std::make_shared<CWorld>(nullptr, engineContext, TerrainPreset{});
		/// PRESET 1 ///
		preset.globalHeightScale = 2.0f;
		preset.base.type = NoiseType::OpenSimplex2;
		preset.base.seed = 3847598;
		preset.base.frequency = 0.009f;
		preset.base.amplitude = 5.0f;
		preset.exponent = 1.7f;

		preset.continentalness.type = NoiseType::Cellular;
		preset.continentalness.amplitude = 0.4f;
		preset.continentalness.frequency = 0.005f;
		preset.continentalness.fractal = FractalType::PingPong;
		preset.continentalness.fractalOctaves = 2;
		preset.continentalness.gain = 0.50f;
		preset.continentalness.lacunarity = 2;
		preset.continentalness.cellType = CellularType::Euclidian;
		preset.continentalness.returnType = ReturnType::Distance2Sub;

		preset.peaksValleys.type = NoiseType::Cubic;
		preset.peaksValleys.cellType = CellularType::Hybrid;
		preset.peaksValleys.returnType = ReturnType::Distance2;
		preset.peaksValleys.fractal = FractalType::Ridged;
		preset.peaksValleys.fractalOctaves = 4;
		preset.peaksValleys.gain = 1.5f;
		preset.peaksValleys.fractalWeightedStrength = 1.8f;
		preset.peaksValleys.amplitude = 0.9f;
		preset.peaksValleys.frequency = 0.9f;

		/// PRESET 2 /// WINNING COMBO SO FAR
		preset2.base.type = NoiseType::Perlin;
		preset2.base.seed = 421322;
		preset2.base.frequency = 0.008f;
		preset2.base.amplitude = 1.0f;
		preset2.base.fractal = FractalType::PingPong;
		preset2.base.fractalOctaves = 5;
		preset2.base.lacunarity = 0.2f;
		preset2.base.gain = 0.5f;
		preset2.exponent = 2.0f;

		preset2.continentalness.type = NoiseType::Cellular;
		preset2.continentalness.seed = 234908;
		preset2.continentalness.frequency = 0.006f;
		preset2.continentalness.fractal = FractalType::Ridged;
		preset2.continentalness.amplitude = 1.0f;
		preset2.continentalness.fractalOctaves = 3;
		preset2.continentalness.gain = 0.90f;
		preset2.continentalness.lacunarity = 2;
		preset2.continentalness.cellType = CellularType::Euclidian;
		preset2.continentalness.returnType = ReturnType::Distance2;

		/// PRESET 3 ///
		preset3.concatenate = true;
		preset3.globalHeightScale = 2.0f;
		preset3.base.type = NoiseType::OpenSimplex2;
		preset3.base.seed = 123;
		preset3.base.frequency = 0.009f;
		preset3.base.amplitude = 5.0f;
		//preset3.base.fractal = FractalType::Ridged;

		preset3.continentalness.type = NoiseType::Cellular;
		preset3.continentalness.frequency = 0.0008f;
		preset3.continentalness.amplitude = 1.0;
		preset3.continentalness.fractal = FractalType::FBm;
		preset3.continentalness.fractalOctaves = 2;
		preset3.continentalness.gain = 0.7f;
		preset3.continentalness.cellType = CellularType::Manhattan;
		preset3.continentalness.returnType = ReturnType::Distance;
		preset3.continentalness.cellularJitter = 1.2f;
		preset3.continentalness.domainWarp = WarpType::OpenSimplex2;
		preset3.continentalness.warpAmplitude = 0.5f;

		preset3.peaksValleys.type = NoiseType::Value;
		preset3.peaksValleys.cellType = CellularType::Hybrid;
		preset3.peaksValleys.returnType = ReturnType::Distance2;
		preset3.peaksValleys.fractal = FractalType::Ridged;
		preset3.peaksValleys.fractalOctaves = 3;
		preset3.peaksValleys.gain = 0.8f;
		preset3.peaksValleys.fractalWeightedStrength = 1.8f;
		preset3.peaksValleys.amplitude = 0.9f;
		preset3.peaksValleys.frequency = 0.09f;
		preset3.exponent = 2.0f;

		//wC->InitializeWorld(&preset3);
		wC->InitializeWorld(0);
		//wC->InitializeWorld(1);
		WorldActor->AddComponent<CWorld>(wC);
		WorldActor->AddComponent<CMaterial>(mat3);
		WorldActor->OnCreate();
		
		actorsInScene.push_back(WorldActor);

		//actorsInScene.push_back(engineContext.assetManager->GetCamera());

		//step 3 Actors being added to the scene.
		camera = engineContext.assetManager->GetCamera();	
		world = WorldActor;
		
	}
		break;

	case RendererType::OPENGL:
		break;
	}

	return true;
}

void Scene3::HandleEvents(const SDL_Event& sdlEvent) {
	
		switch (sdlEvent.type) {
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
		{
			if (sdlEvent.type == SDL_EVENT_KEY_DOWN) {
				// escape stuff
				if (sdlEvent.key.key == SDLK_ESCAPE) {
					mouseLocked = !mouseLocked;
					SDL_SetWindowRelativeMouseMode(dynamic_cast<VulkanRenderer*>(engineContext.renderer)->getWindow(), mouseLocked);
				}
				else if (sdlEvent.key.key == SDLK_1) {
					auto wA = std::dynamic_pointer_cast<CActor>(world);
					auto w = wA->GetComponent<CWorld>();
					w->OnDestroy();
					w->InitializeWorld(0);
				}
				else if (sdlEvent.key.key == SDLK_2) {
					auto wA = std::dynamic_pointer_cast<CActor>(world);
					auto w = wA->GetComponent<CWorld>();
					w->OnDestroy();
					w->InitializeWorld(1);

				}
				else if (sdlEvent.key.key == SDLK_3) {
					auto wA = std::dynamic_pointer_cast<CActor>(world);
					auto w = wA->GetComponent<CWorld>();
					w->OnDestroy();
					w->InitializeWorld(2);
				}

				else if (sdlEvent.key.key == SDLK_B)
				{
					engineContext.fmodController->playsong(AudioState::PAUSE);
				}
				else if (sdlEvent.key.key == SDLK_N)
				{
					engineContext.fmodController->playsong(AudioState::PLAY);
				}
			}
			auto p1 = std::dynamic_pointer_cast<CActor>(camera);
			auto playerController = p1->GetComponent<CInput>();
			if (playerController) {
				playerController->HandleKeyboardInput(sdlEvent);
			}
			break;
		}

		case SDL_EVENT_MOUSE_MOTION:
		{
			auto p1 = std::dynamic_pointer_cast<CActor>(camera);
			auto playerController = p1->GetComponent<CInput>();
			if (playerController) {
				playerController->HandleMouseMotion(sdlEvent);
			}
			break;
		}

		}
	
}

std::vector<MATHEX::Plane> Scene3::GenerateFrustumPLane()
{
	std::vector<MATHEX::Plane> fusturm;
	//Matrix4 proj = camera->getProjectionMatrix() * camera->getViewMatrix();
	auto cam = std::dynamic_pointer_cast<CActor>(camera);
	auto camComp = cam->GetComponent<CCamera>();
	Matrix4 proj = camComp->GetProjectionMatrix() * camComp->GetViewMatrix();
	
	MATHEX::Plane left, right, top, bottom, near, far;
	left.x = proj[3] + proj[0];
	left.y = proj[7] + proj[4];
	left.z = proj[11] + proj[8];	
	left.d = (proj[15] + proj[12]);

	right.x = proj[3] - proj[0];
	right.y = proj[7] - proj[4];
	right.z = proj[11] - proj[8];
	right.d = (proj[15] - proj[12]);

	bottom.x = proj[3] + proj[1];
	bottom.y = proj[7] + proj[5];
	bottom.z = proj[11] + proj[9];
	bottom.d = (proj[15] + proj[13]);

	top.x = proj[3] - proj[1];
	top.y = proj[7] - proj[5];
	top.z = proj[11] - proj[9];
	top.d = (proj[15] - proj[13]);

	near.x = proj[3] + proj[2];
	near.y = proj[7] + proj[6];
	near.z = proj[11] + proj[10];
	near.d = proj[15] + proj[14];

	far.x = proj[3] - proj[2];
	far.y = proj[7] - proj[6];
	far.z = proj[11] - proj[10];
	far.d = (proj[15] - proj[14]);

	// Normalizaiont matters if we care for the actual distance
	// when we do the dot product.
	// if we are just checking below or above 0 then 
	// no need to normalize.
	left = MATHEX::PMath::normalize(left);
	right = MATHEX::PMath::normalize(right);
	bottom = MATHEX::PMath::normalize(bottom);
	top = MATHEX::PMath::normalize(top);
	near = MATHEX::PMath::normalize(near);
	far = MATHEX::PMath::normalize(far);

	fusturm.push_back(left);
	fusturm.push_back(right);
	fusturm.push_back(bottom);
	fusturm.push_back(top);
	fusturm.push_back(near);
	fusturm.push_back(far);
	return fusturm;
}

void Scene3::FrustumCheck()
{
	std::vector<MATHEX::Plane> fusturm = GenerateFrustumPLane();

	auto worldActor = std::dynamic_pointer_cast<CActor>(world);
	auto chunksData = worldActor->GetComponent<CWorld>()->GetChunkRenderData();
	for (auto& c : *chunksData) {
		//auto& c = pair.second;
		c.isCulled = false;

		for (int i = 0; i < 6; i++) {
	
			const MATHEX::Plane& p = fusturm[i];

			Vec3 pVertex;

			pVertex.x = (p.n.x > 0) ? c.aabb.max.x : c.aabb.min.x;
			pVertex.y = (p.n.y > 0) ? c.aabb.max.y : c.aabb.min.y;
			pVertex.z = (p.n.z > 0) ? c.aabb.max.z : c.aabb.min.z;

			float dot = p.n.x * pVertex.x + 
						p.n.y * pVertex.y + 
						p.n.z * pVertex.z;

			if (dot < -p.d) { // Can easly Changed to a Radius of a sphere around the Postion by -r instead of 0
				c.isCulled = true;
				//std::cout << "culled chunk at pos: " << std::endl;
				break;
			}

			// just to check if it intersects probs not needed
			//Vec3 nVertex = Vec3((p.n.x > 0) ? c.aabb.min.x : c.aabb.max.x,
			//					(p.n.y > 0) ? c.aabb.min.y : c.aabb.max.y,
			//					(p.n.z > 0) ? c.aabb.min.z : c.aabb.max.z);

			//float dot2 = p.n.x * nVertex.x + 
			//			 p.n.y * nVertex.y + 
			//			 p.n.z * nVertex.z;

			//if (dot2 <= -p.d) {
			//	c.isCulled = false;
			//}
		}
	}

}



void Scene3::Update(const float deltaTime) {

	// TODO: move to physics system.
	auto player = std::dynamic_pointer_cast<CActor>(camera);
	if (player) {
		auto playerController =player->GetComponent<CInput>();
		auto phys  =player->GetComponent<CPhysics>();
		if (playerController) {
			playerController->UpdateInput(deltaTime);
			phys->Update(deltaTime);
		}
	}
	
	FrustumCheck();
	
	// world collision checking
	// through a system maybe?
	//
	// iterate through the chunk render data
	// keep track 
	// int mapX, mapY; (start at 0)
	// iterate through and when 
	// mapX < WORLD_SIZE
	// mapX = 0;
	// mapY++
	// when mapY < WORLD_SIZE && mapX < WORLD_SIZE -> return;
	// mapX and mapY can be used to grab the correct chunk from chunkMap
	// chunkMap.find(Vec2(mapX, mapY));
	// chunkMap holds indices and vertices on CPU so can be used to check collision with player
	//  
	//
	
}

void Scene3::Render() const {
	switch (engineContext.renderer->getRendererType()) {

	case RendererType::VULKAN:
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);

		
		{
		/*	std::vector<Ref<Component>> drawlist;
			actorsInScene.push_back(plane);*/
			/*drawlist.push_back(actor);
			drawlist.push_back(actor1);
			drawlist.push_back(plane);
			drawlist.push_back(World);*/
			vRenderer->RenderECS(engineContext,actorsInScene);// Context obejct
		}
		break;

	case RendererType::OPENGL:
		OpenGLRenderer* glRenderer;
		glRenderer = dynamic_cast<OpenGLRenderer*>(engineContext.renderer);
		/// Clear the screen
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		/// Draw your scene here
		glUseProgram(0);
		
		break;
	}
}


void Scene3::OnDestroy() {
	VulkanRenderer* vRenderer;
	vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);
	if(vRenderer){
		vkDeviceWaitIdle(vRenderer->getDevice());				
		
	
		vRenderer->DestroyUBO(lightsUBO);
		
		engineContext.fmodController->playsong(0);

		world->OnDestroy();
	
		
		
	}

	
}
