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
		auto wC = std::make_shared<CWorld>(nullptr, engineContext.renderer, TerrainPreset{});

		preset.concatenate = true;
		preset.globalHeightScale = 15.0f;
		preset.base.type = NoiseType::OpenSimplex2;
		preset.base.seed = 3847598;
		preset.base.frequency = 0.009f;
		preset.base.amplitude = 5.0f;
		preset.base.fractal = FractalType::FBm;
		preset.base.fractalOctaves = 2;
		preset.base.gain = 0.3f;
		preset.continentalness.type = NoiseType::Cellular;
		preset.continentalness.frequency = 0.1f;
		preset.continentalness.amplitude = 1.0;
		preset.continentalness.fractal = FractalType::PingPong;
		preset.continentalness.cellType = CellularType::Euclidian;
		preset.continentalness.returnType = ReturnType::Distance; 
		preset.peaksValleys.type = NoiseType::Cubic;
		preset.peaksValleys.cellType = CellularType::Hybrid;
		preset.peaksValleys.returnType = ReturnType::Distance2;
		preset.peaksValleys.fractal = FractalType::PingPong;
		preset.peaksValleys.amplitude = 0.9f;
		preset.peaksValleys.frequency = 9.0f;

		preset.exponent = 1.4f;
		
		
		preset2.base.type = NoiseType::Perlin;
		preset2.base.seed = 421322;	
		preset2.base.frequency = 0.03f;
		preset2.base.amplitude = 5.7f;
		preset2.base.fractal = FractalType::FBm;
		preset2.base.fractalOctaves = 5;
		preset2.base.lacunarity = 0.2f;
		preset2.base.gain = 0.1f;
		preset2.exponent = 2.0f;

		wC->InitializeWorld(&preset);
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
