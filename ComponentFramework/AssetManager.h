#pragma once
#include <fstream>
#include <memory>
#include <string>
#include "json.hpp"
#include "VulkanRenderer.h"
#include <unordered_map>
#include "Component.h"
#include "EngineContext.h"
// link to examples and documantation https://github.com/nlohmann/json?tab=readme-ov-file#examples
//link to exception list https://json.nlohmann.me/home/exceptions/
//helpful video https://www.youtube.com/watch?v=NuWQp_uAvwo&t=119s

class CMesh;
class CMaterial;
class CShader;
class CActor;
class CTransform;
class CGlobalLight;
class CCamera;

class AssetManager
{
private:
	//WORK ON TEMPLATING FUNCTIONS 
	EngineContext engineContext;
	Ref<CActor> camera;
	nlohmann::json jsonLoader;
	std::unordered_map<std::string, std::shared_ptr<Component>> assetMap;
	std::vector<std::shared_ptr<Component>> actorMap;
	template<typename T>
	void assetMapInsert(const std::string id, Ref<T> asset)
	{
		assetMap[id] = asset;
	};
	template<typename T>
	Ref<T> assetMapGet(const std::string id)
	{
		auto checker = assetMap.find(id);
		Ref<Component> result = checker->second;


		if (checker != assetMap.end())
		{
			return std::dynamic_pointer_cast<T>(result);
		
		}
		return nullptr;
	};
	//give all components a custom id.
public:
	AssetManager(){};
	~AssetManager();
	bool set(EngineContext context_)
	{
		engineContext = context_;
		return true;
	}
	bool LoadAsset(const std::string& filepath_);
	bool CreateActor(const std::string& actorId, Ref<CMesh> mesh_, Ref<CMaterial> tex_, Ref<CShader> shader_);
	std::vector<Ref<Component>> GetActorsInScene();
	Ref<Component> GetCamera();
	Ref<CMesh> GetMesh(const std::string& id);
	Ref<CMaterial> GetMat(const std::string& id);
	Ref<CShader> GetShader(const std::string& id);
	
};