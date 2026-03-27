#pragma once
//#include "AssetManager.h"
//#include "Renderer.h"


class Renderer;
class AssetManager;
class FmodController;
class SYS_Light;
class VkImGUISystem;
class PhysicsManager;

struct EngineContext
{
    Renderer* renderer = nullptr;
    AssetManager* assetManager = nullptr;
	FmodController* fmodController = nullptr;
    SYS_Light* lightSys = nullptr;
    VkImGUISystem* VKImGUI = nullptr;
	PhysicsManager* physicsManager = nullptr;
    void Set(
        Renderer& renderer_, 
        AssetManager& assetManager_, 
        FmodController& fmodController_,
        SYS_Light& LightSys_, 
        VkImGUISystem& imgui,
		PhysicsManager& physicsManager_
        )
    {
        renderer = &renderer_;
        assetManager = &assetManager_;
        fmodController = &fmodController_;
        lightSys = &LightSys_;
        VKImGUI = &imgui;
		physicsManager = &physicsManager_;
    }
};