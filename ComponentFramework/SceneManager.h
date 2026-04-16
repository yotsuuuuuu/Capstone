#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <string>
#include "Renderer.h"
#include "EngineContext.h"
class SceneManager  {
public:
	
	SceneManager();
	~SceneManager();
	void Run();
	bool Initialize(std::string name_, int width_, int height_);
	void GetEvents();
		// assetManger* getAMgern();  -> get load
		// Ref<AssetMange> 
		// ref<comp> getActor("ke") ;  assetManger -> get actor key
		// vulk render -> created global stuff (asstmanger get info);
		// create manger load vulkab global"
	
private:
	
	
	enum SCENE_NUMBER {
		SCENE0 = 0,
		SCENE1,
		SCENE2,
		SCENE3,
		SCENE4,
		SCENE5,
		SCENE6
	};

	enum class RendererType rendererType;
	class Scene* currentScene;
	class Timer* timer;

	EngineContext engineContext;
	Renderer* renderer;
	AssetManager* assetManager;
	AudioManager* audioManager;
	SYS_Light* LightSystem;
	VkImGUISystem* VKImGui;
	PhysicsManager* physicsManager;

	unsigned int fps;
	bool isRunning;
	bool isWindowMinimized = false;
	bool needSwapChainRecreation = false;
	void BuildScene(SCENE_NUMBER scene_);
};


#endif // SCENEMANAGER_H