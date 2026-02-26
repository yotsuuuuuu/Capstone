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
#include "CWorld.h"
#include "CInput.h"
#include "CCamera.h"
#include "CPhysics.h"
#include "CGlobalLight.h"
#include "VulkanRenderer.h"
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
	int width = 0, height = 0;
	float aspectRatio;
	switch (engineContext.renderer->getRendererType()){
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);		
		
		actorsInScene = engineContext.assetManager->GetActorsInScene();
		//step 1.3 Materials

		 std::vector<std::string> filepaths = { "./textures/rock.png" };

		Ref<CMaterial> mat3 = std::make_shared<CMaterial>(nullptr, engineContext.renderer, filepaths, engineContext.assetManager->GetShader("main"));
		mat3->OnCreate();

		// step 2 create actors
		


		Ref<CActor> WorldActor = std::make_shared<CActor>(nullptr);
		auto wC = std::make_shared<CWorld>(nullptr, engineContext, TerrainPreset{});

		//preset.concatenate = true;
		preset.globalHeightScale = 2.0f;
		preset.base.type = NoiseType::OpenSimplex2;
		preset.base.seed = 3847598;
		preset.base.frequency = 0.009f;
		preset.base.amplitude = 15.0f;
		preset.exponent = 1.4f;
		preset.base.fractal = FractalType::FBm;
		preset.base.lacunarity = 6;
		preset.base.fractalOctaves = 3;
		preset.continentalness.type = NoiseType::Cellular;
		preset.continentalness.amplitude = 0.4f;
		preset.continentalness.frequency = 0.005f;
		preset.continentalness.fractal = FractalType::PingPong;
		preset.continentalness.fractalOctaves = 9;
		preset.continentalness.gain = 0.10f;
		preset.continentalness.lacunarity = 2;
		preset.continentalness.fractalWeightedStrength = 15.090f;
		preset.continentalness.cellType = CellularType::Euclidian;
		preset.continentalness.returnType = ReturnType::Distance2;

		//preset.base.domainWarp = WarpType::OpenSimplex2;



		preset2.base.type = NoiseType::Perlin;
		preset2.base.seed = 421322;
		preset2.base.frequency = 0.008f;
		preset2.base.amplitude = 1.0f;
		preset2.base.fractal = FractalType::FBm;
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
		preset2.continentalness.fractalWeightedStrength = 20.0f;
		preset2.continentalness.cellType = CellularType::Euclidian;
		preset2.continentalness.returnType = ReturnType::Distance2;

		preset3.concatenate = true;
		preset3.globalHeightScale = 2.0f;
		preset3.base.type = NoiseType::OpenSimplex2;
		preset3.base.seed = 3847598;
		preset3.base.frequency = 0.009f;
		preset3.base.amplitude = 5.0f;
		preset3.base.fractal = FractalType::FBm;
		preset3.base.fractalOctaves = 2;
		preset3.base.gain = 0.3f;
		preset3.continentalness.type = NoiseType::Cellular;
		preset3.continentalness.frequency = 0.002f;
		preset3.continentalness.amplitude = 1.6;
		preset3.continentalness.fractal = FractalType::FBm;
		preset3.continentalness.gain = 0.9f;
		preset3.continentalness.fractalWeightedStrength = 15.0f;
		preset3.continentalness.cellType = CellularType::Euclidian;
		preset3.continentalness.returnType = ReturnType::Distance;
		preset3.peaksValleys.type = NoiseType::Cubic;
		preset3.peaksValleys.cellType = CellularType::Hybrid;
		preset3.peaksValleys.returnType = ReturnType::Distance2;
		preset3.peaksValleys.fractal = FractalType::PingPong;
		preset3.peaksValleys.amplitude = 5.9f;
		preset3.peaksValleys.frequency = 0.05f;

		preset3.exponent = 1.4f;

		wC->InitializeWorld(&preset3);
		WorldActor->AddComponent<CWorld>(wC);
		WorldActor->AddComponent<CMaterial>(mat3);
		WorldActor->OnCreate();
		
		actorsInScene.push_back(WorldActor);
	
		
		//step 3 Actors being added to the scene.
		camera = engineContext.assetManager->GetCamera();	
		world = WorldActor;
		engineContext.fmodController->playsong(0);
		
	}
		break;

	case RendererType::OPENGL:
		break;
	}

	return true;
}

void Scene3::HandleEvents(const SDL_Event& sdlEvent) {
	
		switch (sdlEvent.type) {
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
			printf("size changed %d %d\n", sdlEvent.window.data1, sdlEvent.window.data2);
			float aspectRatio = static_cast<float>(sdlEvent.window.data1) / static_cast<float>(sdlEvent.window.data2);
			///camera->Perspective(45.0f, aspectRatio, 0.5f, 20.0f);
			if (engineContext.renderer->getRendererType() == RendererType::VULKAN)
			{
				dynamic_cast<VulkanRenderer*>(engineContext.renderer)->RecreateSwapChain();
			}
			break;
		}
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
		{
			if (sdlEvent.type == SDL_EVENT_KEY_DOWN) {
				// escape stuff
				if (sdlEvent.key.key == SDLK_ESCAPE) {
					mouseLocked = !mouseLocked;
					SDL_SetWindowRelativeMouseMode(dynamic_cast<VulkanRenderer*>(engineContext.renderer)->getWindow(), mouseLocked);
				}
				else if (sdlEvent.key.key == SDLK_9) {
					auto wA = std::dynamic_pointer_cast<CActor>(world);
					auto w = wA->GetComponent<CWorld>();
					w->OnDestroy();
					w->InitializeWorld(&preset);
				}
				else if (sdlEvent.key.key == SDLK_0) {
					auto wA = std::dynamic_pointer_cast<CActor>(world);
					auto w = wA->GetComponent<CWorld>();
					w->OnDestroy();
					w->InitializeWorld(&preset2);

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
void Scene3::Update(const float deltaTime) {
	auto player = std::dynamic_pointer_cast<CActor>(camera);
	if (player) {
		auto playerController =player->GetComponent<CInput>();
		auto phys  =player->GetComponent<CPhysics>();
		if (playerController) {
			playerController->UpdateInput(deltaTime);
			phys->Update(deltaTime);
		}
	}
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
			vRenderer->RenderECS(actorsInScene);// Context obejct
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
