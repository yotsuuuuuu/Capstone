#pragma once
#include "AssetManager.h"
#include "VulkanRenderer.h"
struct EngineContext
{
	VulkanRenderer& renderer;
	AssetManager& assetManager;
	EngineContext(VulkanRenderer& renderer_, AssetManager& assetManger_) : renderer(renderer_), assetManager(assetManger_) {};
};

