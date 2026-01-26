#pragma once
#include <fstream>
#include <memory>
#include <string>
#include "json.hpp"
#include "VulkanRenderer.h"
#include <unordered_map>
// link to examples and documantation https://github.com/nlohmann/json?tab=readme-ov-file#examples
//link to exception list https://json.nlohmann.me/home/exceptions/
//helpful video https://www.youtube.com/watch?v=NuWQp_uAvwo&t=119s

template<typename T>
using Ref = std::shared_ptr<T>;

class CMesh;
class CMaterial;
class CShader;
class CActor;
class AssetManager
{
private:
	VulkanRenderer* renderer;
	nlohmann::json jsonLoader;
	std::unordered_map<std::string, std::shared_ptr<CMesh>> meshMap;
	std::unordered_map<std::string, std::shared_ptr<CMaterial>> materialMap;
	std::unordered_map<std::string, std::shared_ptr<CShader>> shaderMap;
	std::unordered_map<std::string, std::shared_ptr<CActor>> actorMap;
public:
	AssetManager(VulkanRenderer* renderer_):renderer(renderer_) {};
	bool LoadAsset(const std::string& filepath_);
	bool CreateActor(const std::string& actorId, Ref<CMesh> mesh_, Ref<CMaterial> tex_, Ref<CShader> shader_);
	Ref<CMesh> GetMesh(const std::string& id);
	Ref<CMaterial> GetMat(const std::string& id);
	Ref<CShader> GetShader(const std::string& id);
};