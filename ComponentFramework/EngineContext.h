#pragma once
//#include "AssetManager.h"
//#include "Renderer.h"


class Renderer;
class AssetManager;
class FmodController;
class SYS_Light;

struct EngineContext
{
    Renderer* renderer = nullptr;
    AssetManager* assetManager = nullptr;
	FmodController* fmodController = nullptr;
    SYS_Light* lightSys = nullptr;

    void Set(Renderer& renderer_, AssetManager& assetManager_, FmodController& fmodController_,SYS_Light& LightSys_)
    {
        renderer = &renderer_;
        assetManager = &assetManager_;
        fmodController = &fmodController_;
        lightSys = &LightSys_;
    }
};