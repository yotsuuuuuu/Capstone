#pragma once
#include "AssetManager.h"
#include "Renderer.h"
#include "cassert"
struct EngineContext
{
    Renderer* renderer = nullptr;
    AssetManager* assetManager = nullptr;

    void Set(Renderer& renderer_, AssetManager& assetManager_)
    {
        assert(renderer == nullptr && assetManager == nullptr);
        renderer = &renderer_;
        assetManager = &assetManager_;
    }
};