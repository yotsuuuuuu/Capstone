#include <SDL3/SDL.h>
#include "SceneManager.h"
#include "VulkanRenderer.h"
#include "OpenGLRenderer.h"
#include "AssetManager.h"
#include "FmodController.h"
#include "VkImGUISystem.h"
#include "SYS_Light.h"
#include "PhysicsManager.h"

#include "Timer.h"
#include "Scene0.h"
#include "Scene2.h"
#include "Scene3.h"
#include "Debug.h"

SceneManager::SceneManager() :
	currentScene(nullptr), timer(nullptr),
	fps(60), isRunning(false), rendererType(RendererType::VULKAN),
	renderer(nullptr),LightSystem(nullptr),fmodController(nullptr), assetManager(nullptr), VKImGui(nullptr){}

SceneManager::~SceneManager() {
	if (currentScene) {
		currentScene->OnDestroy();
		delete currentScene;
		currentScene = nullptr;
	}
	
	if (timer) {
		delete timer;
		timer = nullptr;
	}
	delete assetManager;
	dynamic_cast<VulkanRenderer*>(renderer)->DestroyGlobalResources();
	
	if (LightSystem) {
		LightSystem->ShutDonw();
		delete LightSystem;
	}
	if (VKImGui) {
		VKImGui->ShutDonw();
		delete VKImGui;
	}
	renderer->OnDestroy();
	engineContext.renderer = nullptr;
	engineContext.assetManager = nullptr;	
	engineContext.fmodController = nullptr;
	engineContext.physicsManager = nullptr;

	delete physicsManager;
	delete fmodController;
	delete renderer;

	Debug::Info("Deleting the GameSceneManager", __FILE__, __LINE__);

}

bool SceneManager::Initialize(std::string name_, int width_, int height_) {

	switch(rendererType){
	case RendererType::OPENGL:
		renderer = new OpenGLRenderer();
		renderer->setRendererType(RendererType::OPENGL);
		renderer->CreateWindow(name_, width_, height_);
		renderer->OnCreate();
			
		
		break;

	case RendererType::VULKAN:
		renderer = new VulkanRenderer();
		renderer->setRendererType(RendererType::VULKAN);
		renderer->CreateWindow(name_, width_, height_);
		renderer->OnCreate();
		break;

	case RendererType::DIRECTX11:
	case RendererType::DIRECTX12:
		Debug::FatalError("Renderer not yet supported", __FILE__, __LINE__);
		return false;
		break;
	}

	timer = new Timer();
	if (timer == nullptr) {
		Debug::FatalError("Failed to initialize Timer object", __FILE__, __LINE__);
		return false;
	}
	fmodController = new FmodController();
	LightSystem = new SYS_Light(&engineContext, 1000);

	VKImGui = new VkImGUISystem();

	assetManager = new AssetManager();
	if (!fmodController->AddSonginFile())
	{
		Debug::Error("Failed to add songs in file to the list", __FILE__, __LINE__);
	}
	if (!fmodController->createSystem())
	{
		Debug::Error("Failed to create fmod system", __FILE__, __LINE__);
	}
	fmodController->InitilizeSongs();

	physicsManager = new PhysicsManager();

	engineContext.Set(*renderer, *assetManager, *fmodController, *LightSystem, *VKImGui, *physicsManager);
	assetManager->set(engineContext);
	engineContext.assetManager->LoadCamera("./test.json");		

	auto cntx = static_cast<VulkanRenderer*>(renderer)->GetImGuiContext();
	if (!VKImGui->Initialize(cntx)) {
		Debug::FatalError("Failed to initialize ImGui System", __FILE__, __LINE__);
		return false;
	}

	if (!LightSystem->Initilize()) {
		Debug::FatalError("Failed to initialize Light System", __FILE__, __LINE__);
		return false;
	}
	LightSystem->ComputeClusters();


	if (!static_cast<VulkanRenderer*>(renderer)->CreateGlobalRources(engineContext)) {
		return false;
	}
	engineContext.assetManager->LoadAsset("./test.json");
	engineContext.fmodController->AnalyzeAudioOffline(0);
	BuildScene(SCENE3);
	
	return true;
}


void SceneManager::Run() {
	
	timer->Initialize(fps);
	isRunning = true;
	while (isRunning) {		
		timer->StartFrameTime();	

		GetEvents();
		if (currentScene && !isWindowMinimized) {
			currentScene->Update(timer->getDeltaTime());
			//engineContext.fmodController->AnalyzeAudioOnline();
			engineContext.VKImGUI->TestUI();
			currentScene->Render();
		}

		timer->EndFrameTime();
	}
}

void SceneManager::GetEvents() {
	SDL_Event sdlEvent;
	std::string songnametest;
	while (SDL_PollEvent(&sdlEvent)) {
		switch (sdlEvent.type) {
		case SDL_EVENT_QUIT:
			isRunning = false;
			return;
		case SDL_EVENT_KEY_DOWN:
			switch (sdlEvent.key.scancode) {
			case SDL_SCANCODE_Q:
				isRunning = false;
				return;
			case SDL_SCANCODE_F1:
				BuildScene(SCENE0);
				break;
			case SDL_SCANCODE_F2:
				BuildScene(SCENE2);
				break;
			case SDL_SCANCODE_F3:
				BuildScene(SCENE3);
				break;
			case SDL_SCANCODE_F4:
				///BuildScene(SCENE4);
				break;
			case SDL_SCANCODE_F5:
				///BuildScene(SCENE5);
				break;
			case SDL_SCANCODE_F6:
				///BuildScene(SCENE6);
				break;
			case SDL_SCANCODE_EQUALS:
				engineContext.fmodController->Volume(10);
				break;
			case SDL_SCANCODE_MINUS:
				engineContext.fmodController->Volume(-10);
				break;
			case SDL_SCANCODE_KP_0:
				engineContext.fmodController->playsong(0);
				songnametest = engineContext.fmodController->getSongName(0);
				break;
			case SDL_SCANCODE_KP_1:
				songnametest = engineContext.fmodController->getSongName(1);
				engineContext.fmodController->playsong(1);
				break;
			default:
				break;
			}
			break;

		case SDL_EVENT_WINDOW_MINIMIZED:
			isWindowMinimized = true;
			break;

		case SDL_EVENT_WINDOW_RESTORED:
			isWindowMinimized = false;
			needSwapChainRecreation = true;
			break;

		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			needSwapChainRecreation = true;
			break;

		default:
			break;
		}
		
		engineContext.VKImGUI->ImGUIHandelEvents(sdlEvent);
		if (currentScene) currentScene->HandleEvents(sdlEvent);
	}
	if (needSwapChainRecreation) {

		if (engineContext.renderer->getRendererType() == RendererType::VULKAN)
		{
			VulkanRenderer* Vkrender = dynamic_cast<VulkanRenderer*>(engineContext.renderer);
			int width = 0, height = 0;
			SDL_GetWindowSize(Vkrender->getWindow(), &width, &height);
			printf("size changed %d %d\n", width, height);
			float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
			Vkrender->RecreateSwapChain();// recreate swapchains
			engineContext.assetManager->ScreenResizeCameraEvent(aspectRatio); // update main camera
			engineContext.assetManager->RecreatedPipelines(); //recreate Pipeliens
			//update Light System
			engineContext.lightSys->ScreenResizeCameraEvent(width, height);
			engineContext.lightSys->ComputeClusters();
			//update imgui
			auto cntx = Vkrender->GetImGuiContext();
			engineContext.VKImGUI->ShutDonw();
			engineContext.VKImGUI->Initialize(cntx);
			needSwapChainRecreation = false;
		}

	}

	if (currentScene == nullptr) {
		Debug::FatalError("Failed to initialize Scene", __FILE__, __LINE__);
		isRunning = false;
		return;
	}
}

void SceneManager::BuildScene(SCENE_NUMBER scene) {
	bool status; 

	if (currentScene != nullptr) {
		currentScene->OnDestroy();
		delete currentScene;
		currentScene = nullptr;
	}

	switch (scene) {
	case SCENE0:  
		currentScene = new Scene0(engineContext);
				status = currentScene->OnCreate();
		break;

	case SCENE1:
		///currentScene = new Scene1();
		//status = currentScene->OnCreate();
		break;

	case SCENE2:
		//currentScene = new Scene2(engineContext);
		//status = currentScene->OnCreate();
		break;

	case SCENE3:
		currentScene = new Scene3(engineContext);
		status = currentScene->OnCreate();
		break;

	case SCENE4:
		///currentScene = new Scene4();
		//status = currentScene->OnCreate();
		break;

	case SCENE5:
		///currentScene = new Scene5();
		status = currentScene->OnCreate();
		break;

	case SCENE6:
		///currentScene = new Scene6();
		status = currentScene->OnCreate();
		break;

	default:
		Debug::Error("Incorrect scene number assigned in the manager", __FILE__, __LINE__);
		currentScene = nullptr;
		break;
	}	
}

