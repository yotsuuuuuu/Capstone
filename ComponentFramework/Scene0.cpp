#include <glew.h>
#include <iostream>
#include "Debug.h"
#include "Scene0.h"
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

Scene0::Scene0(EngineContext& context_): 
	Scene(context_) {
	Debug::Info("Created Scene0: ", __FILE__, __LINE__);
}

Scene0::~Scene0() {
}

bool Scene0::OnCreate() {
	int width = 0, height = 0;
	float aspectRatio;
	switch (engineContext.renderer->getRendererType()){
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);
		
		

		SDL_GetWindowSize(vRenderer->getWindow(), &width, &height);
		aspectRatio = static_cast<float>(width) / static_cast<float>(height);	
		
		// step 1 Create the  GLOBAL componetes
		
		actorsInScene = engineContext.assetManager->GetActorsInScene();			
		

		std::vector<std::string> filepaths = { "./textures/rock.png" };

		Ref<CMaterial> mat3 = std::make_shared<CMaterial>(nullptr, engineContext.renderer, filepaths, engineContext.assetManager->GetShader("Terrain"));
		mat3->OnCreate();

		Ref<CActor> WorldActor = std::make_shared<CActor>(nullptr);
		auto wC = std::make_shared<CWorld>(nullptr, engineContext, TerrainPreset{});
		TerrainPreset preset;

		preset.concatenate = true;
		//preset.globalHeightScale = 5.0f;
		preset.base.type = NoiseType::OpenSimplex2;
		preset.base.seed = 42;
		preset.base.frequency = 0.009f;
		preset.base.amplitude = 2.0f;
		preset.base.fractal = FractalType::FBm;
		preset.base.fractalOctaves = 5;
		preset.base.gain = 0.3f;

		wC->InitializeWorld(&preset);
		WorldActor->AddComponent<CWorld>(wC);
		WorldActor->AddComponent<CMaterial>(mat3);
		WorldActor->OnCreate();
		//step 3 Actors being added to the scene.
		World = WorldActor;
		//engineContext.fmodController->playsong(0);

		actorsInScene.push_back(World);

	}
		break;

	case RendererType::OPENGL:
		break;
	}

	return true;
}

void Scene0::HandleEvents(const SDL_Event& sdlEvent) {
	
		switch (sdlEvent.type) {
	
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
		{

			// escape stuff
			if (sdlEvent.key.key == SDLK_ESCAPE && sdlEvent.type == SDL_EVENT_KEY_DOWN) {
				mouseLocked = !mouseLocked;
				SDL_SetWindowRelativeMouseMode(dynamic_cast<VulkanRenderer*>(engineContext.renderer)->getWindow(), mouseLocked);
			}

			auto p1 = std::dynamic_pointer_cast<CActor>(engineContext.assetManager->GetCamera());
			auto playerController = p1->GetComponent<CInput>();
			if (playerController) {
				playerController->HandleKeyboardInput(sdlEvent);
			}
			break;
		}

		case SDL_EVENT_MOUSE_MOTION:
		{
			auto p1 = std::dynamic_pointer_cast<CActor>(engineContext.assetManager->GetCamera());
			auto playerController = p1->GetComponent<CInput>();
			if (playerController) {
				playerController->HandleMouseMotion(sdlEvent);
			}
			break;
		}

		}
	
}
void Scene0::Update(const float deltaTime) {
	auto player = std::dynamic_pointer_cast<CActor>(engineContext.assetManager->GetCamera());
	if (player) {
		auto playerController =player->GetComponent<CInput>();
		auto phys  =player->GetComponent<CPhysics>();
		if (playerController) {
			playerController->UpdateInput(deltaTime);
			phys->Update(deltaTime);
		}
	}
}

void Scene0::Render() const {
	switch (engineContext.renderer->getRendererType()) {

	case RendererType::VULKAN:
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);

		
		{
		
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


void Scene0::OnDestroy() {
	VulkanRenderer* vRenderer;
	vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);
	if(vRenderer){
		vkDeviceWaitIdle(vRenderer->getDevice());				
		
		World->OnDestroy();
		
	
		
	}

	
}
