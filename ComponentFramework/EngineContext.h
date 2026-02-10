#pragma once
//#include "AssetManager.h"
//#include "Renderer.h"
#include "cassert"

class Renderer;
class AssetManager;

struct EngineContext
{
    Renderer* renderer = nullptr;
    AssetManager* assetManager = nullptr;

    void Set(Renderer& renderer_, AssetManager& assetManager_)
    {
        //assert(&renderer_ == nullptr && &assetManager_ == nullptr);
        renderer = &renderer_;
        assetManager = &assetManager_;
    }
};