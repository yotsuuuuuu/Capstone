#pragma once
//#include "AssetManager.h"
//#include "Renderer.h"


class Renderer;
class AssetManager;
class AudioManager;
class SYS_Light;
class VkImGUISystem;
class PhysicsManager;

struct EngineContext
{
    Renderer* renderer = nullptr;
    AssetManager* assetManager = nullptr;
	AudioManager* audioManager = nullptr;
    SYS_Light* lightSys = nullptr;
    VkImGUISystem* VKImGUI = nullptr;
	PhysicsManager* physicsManager = nullptr;
    void Set(
        Renderer& renderer_, 
        AssetManager& assetManager_, 
        AudioManager& audioManager_,
        SYS_Light& LightSys_, 
        VkImGUISystem& imgui,
		PhysicsManager& physicsManager_
        )
    {
        renderer = &renderer_;
        assetManager = &assetManager_;
        audioManager = &audioManager_;
        lightSys = &LightSys_;
        VKImGUI = &imgui;
		physicsManager = &physicsManager_;
    }
};