#pragma once
//#include "AssetManager.h"
//#include "Renderer.h"


class Renderer;
class AssetManager;
class FmodController;

struct EngineContext
{
    Renderer* renderer = nullptr;
    AssetManager* assetManager = nullptr;
	FmodController* fmodController = nullptr;

    void Set(Renderer& renderer_, AssetManager& assetManager_, FmodController& fmodController_)
    {
        renderer = &renderer_;
        assetManager = &assetManager_;
        fmodController = &fmodController_;
    }
};