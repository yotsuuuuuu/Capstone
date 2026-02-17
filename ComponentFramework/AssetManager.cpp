#include "AssetManager.h"
#include "CActor.h"
#include "CMesh.h"
#include "CMaterial.h"
#include "CShader.h"
#include "CTransform.h"
#include "Component.h"

using json = nlohmann::json;

bool AssetManager::LoadAsset(const std::string& filepath_)
{
    std::ifstream file(filepath_);
    if (!file.is_open())
    {
        std::cerr << "Failed to open asset database: " << filepath_ << "\n";
        return false;
    }
    jsonLoader = json::parse(file);

	if (!jsonLoader.contains("Meshes"))
	{
		std::cout << "json does not contain meshes" << "\n";
		return false;
	}

	for (auto& [meshId, meshPath] : jsonLoader["Meshes"].items())
	{
		Ref<CMesh> mesh = std::make_shared<CMesh>(nullptr, renderer, meshPath.get<std::string>());
        
        if (!mesh->OnCreate())
        {
			std::cout << "Failed to create mesh :" << meshId << "\n";
        }

		assetMap[meshId] = mesh;
	}

	if (!jsonLoader.contains("Textures")) 
    {
		std::cout << "json does not contain textures" << "\n";
		return false;
	}

	if (!jsonLoader.contains("Shaders"))
	{
		std::cout << "json does not contain shader" << "\n";
		return false;
	}

	for(auto & [shaderId, shaderData] : jsonLoader["Shaders"].items())
	{
		std::vector<SingleDescriptorSetLayoutInfo> layoutInfo;
		std::pair<std::string, std::string> shaderPaths;
		shaderPaths.first = shaderData["frag"].get<std::string>();
		shaderPaths.second = shaderData["vert"].get<std::string>();
		int shaderBinding = shaderData["binding"].get<int>();
		int shaderType = shaderData["type"].get<int>();
		int shaderStage = shaderData["stage"].get<int>();
		renderer->AddToDescriptorLayoutCollection(layoutInfo, shaderBinding, static_cast<VkDescriptorType>(shaderType), static_cast<VkShaderStageFlagBits>(shaderStage), 1);
		Ref<CShader> cshade = std::make_shared<CShader>(nullptr, renderer, layoutInfo, shaderPaths.second, shaderPaths.first);
        if (!cshade->OnCreate()) 
        {
			std::cout << "Failed to create shader :" << shaderId << "\n";
        }
		assetMap[shaderId] = cshade;
	}

    if (!jsonLoader.contains("Material")) 
    {
        std::cout << "json does not contain material" << "\n";
        return false;
    }

    for (auto& [matId, matData] : jsonLoader["Material"].items())
    {

        std::vector<std::string> texName;
		std::string filepath = matData["texture"].get<std::string>();
		texName.push_back(jsonLoader["Textures"][filepath].get<std::string>());
		Ref<CMaterial> mat1 = std::make_shared<CMaterial>(nullptr, renderer,texName, assetMapGet<CShader>(matData["shader"]));
        assetMap[matId] = mat1;
        
        if (!mat1->OnCreate())
        {
			std::cout << "Failed to create material :"  << matId << "\n";
        }

    }

	if (!jsonLoader.contains("Actor"))
	{
		std::cout << "json does not contain an actor section " << "\n";
		return false;
	}

	for (auto& [actorname, actorData] : jsonLoader["Actor"].items())
	{
		Ref<CActor> act = std::make_shared<CActor>(nullptr);
        auto pos = actorData["Transform"]["position"].get<std::vector<float>>();
        auto rot = actorData["Transform"]["rotation"].get<std::vector<float>>();

		Ref<CTransform> t = std::make_shared<CTransform>(nullptr, Vec3(pos[0], pos[1], pos[2]), QMath::angleAxisRotation(rot[0],Vec3(rot[1],rot[2],rot[3])), Vec3(1, 1, 1));

		act->AddComponent<CTransform>(t);
        Ref<CMesh> temp = assetMapGet<CMesh>(actorData["Mesh"].get<std::string>());
		act->AddComponent<CMesh>(temp);
		act->AddComponent<CMaterial>(assetMapGet<CMaterial>(actorData["Mat"].get<std::string>()));

        if (!act->OnCreate())
        {
			std::cout << "Failed to create actor :" << actorname << "\n";
        }
		actorMap.push_back(act);
		
	}

    return true;
}

bool AssetManager::CreateActor(const std::string& actorId, Ref<CMesh> mesh_, Ref<CMaterial> tex_, Ref<CShader> shader_)
{
    return false;
}

std::vector<std::shared_ptr<Component>> AssetManager::GetActorsInScene()
{
    return actorMap;
}

Ref<CMesh> AssetManager::GetMesh(const std::string& id)
{
    auto checker = assetMapGet<CMesh>(id);

    if (checker)
    {
		return checker;
    }
    if (!jsonLoader.contains("Meshes") || !jsonLoader["Meshes"].contains(id))
    {
        std::cout << "json does not contain meshes" << id << "\n";
        return nullptr;
    }
    std::string meshpath = jsonLoader["Meshes"][id].get<std::string>();
    Ref<CMesh> mesh = std::make_shared<CMesh>(nullptr,renderer,meshpath);
    assetMap[id] = mesh;
    return mesh;
}

Ref<CMaterial> AssetManager::GetMat(const std::string& id)
{
	auto checker = assetMapGet<CMaterial>(id); 
    if (checker)
    {
        return checker;
    }

    if (!jsonLoader.contains("Material") || !jsonLoader["Material"].contains(id))
    {
        std::cout << "json does not contain meshes" << id << "\n";
        return nullptr;
    }

    std::vector<std::string> filepathtexture;
    filepathtexture.push_back(jsonLoader["Material"][id]["texture"].get<std::string>());
    Ref<CMaterial> mat1 = std::make_shared<CMaterial>(nullptr, renderer, filepathtexture, GetShader(jsonLoader["Material"][id]["shader"]));
    assetMap[id] = mat1;
    mat1->OnCreate();

    return mat1;
}

Ref<CShader> AssetManager::GetShader(const std::string& id)
{
    //grab thje vert and frag seprately based on what shader they want
    //so if they say phong itll filter phong first then look for its vert and frag and combo that
	auto checker = assetMapGet<CShader>(id);
    if (checker)
    {
        return checker;
    }

    if (!jsonLoader.contains("Shaders") || !jsonLoader["Shaders"].contains(id))
    {
        std::cout << "json does not contain shader" << id << "\n";
        return nullptr;
    }

    std::vector<SingleDescriptorSetLayoutInfo> layoutInfo;
    std::pair<std::string, std::string> shaderPaths;
    shaderPaths.first = jsonLoader["Shaders"][id]["frag"].get<std::string>();
    shaderPaths.second = jsonLoader["Shaders"][id]["vert"].get<std::string>();

    int shaderBinding = jsonLoader["Shaders"][id]["binding"].get<int>();
    int shaderType = jsonLoader["Shaders"][id]["type"].get<int>();
    int shaderStage = jsonLoader["Shaders"][id]["stage"].get<int>();

    renderer->AddToDescriptorLayoutCollection(layoutInfo, shaderBinding, static_cast<VkDescriptorType>(shaderType), static_cast<VkShaderStageFlagBits>(shaderStage), 1);
    Ref<CShader> cshade = std::make_shared<CShader>(nullptr, renderer, layoutInfo, shaderPaths.second, shaderPaths.first);
    assetMap[id] = cshade;
    return cshade;
}
AssetManager::~AssetManager()
{
	for (auto& [key, asset] : assetMap)
	{
		asset->OnDestroy();
	}
	assetMap.clear();

    for(size_t i = 0; i < actorMap.size();i++)
	{
		actorMap[i]->OnDestroy();
	}
	actorMap.clear();

	renderer = nullptr;
}