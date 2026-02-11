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
    return true;
}

bool AssetManager::CreateActor(const std::string& actorId, Ref<CMesh> mesh_, Ref<CMaterial> tex_, Ref<CShader> shader_)
{
    return false;
}

std::vector<std::shared_ptr<Component>> AssetManager::GetActorsInScene()
{
    if (!jsonLoader.contains("Actor"))
    {
        std::cout << "json does not contain an actor section " << "\n";
        /*return std::vector<>;*/
    }
    Ref<CActor> act = std::make_shared<CActor>(nullptr);
    for ( auto&[actorname,actorData] : jsonLoader["Actor"].items())
    {
        
        //act->AddComponent(GetMesh(actorData["Mesh"])); //doesnt work
        //act->AddComponent(GetMat(actorData["Mat"]));
        Ref<CTransform> t = std::make_shared<CTransform>(nullptr, Vec3(-1, 0, 0), Quaternion(), Vec3(1, 1, 1));
        act->AddComponent<CTransform>(t); 
        actorMap.push_back(act);
    }
    return actorMap;
}

Ref<CMesh> AssetManager::GetMesh(const std::string& id)
{
    auto checker = meshMap.find(id);
    if (checker != meshMap.end())
    {
        return checker->second;
    }

    if (!jsonLoader.contains("Meshes") || !jsonLoader["Meshes"].contains(id))
    {
        std::cout << "json does not contain meshes" << id << "\n";
        return nullptr;
    }
    std::string meshpath = jsonLoader["Meshes"][id].get<std::string>();
    Ref<CMesh> mesh = std::make_shared<CMesh>(nullptr,renderer,meshpath);
    meshMap[id] = mesh;
    return mesh;
}

Ref<CMaterial> AssetManager::GetMat(const std::string& id)
{
    auto checker = materialMap.find(id);
    if (checker != materialMap.end())
    {
        return checker->second;
    }

    if (!jsonLoader.contains("Material") || !jsonLoader["Material"].contains(id))
    {
        std::cout << "json does not contain meshes" << id << "\n";
        return nullptr;
    }

    std::vector<std::string> filepathtexture;
    filepathtexture.push_back(jsonLoader["Material"][id]["texture"].get<std::string>());
    Ref<CMaterial> mat1 = std::make_shared<CMaterial>(nullptr, renderer, filepathtexture, GetShader(jsonLoader["Material"][id]["shader"]));
    materialMap[id] = mat1;
    mat1->OnCreate();

    return mat1;
}

Ref<CShader> AssetManager::GetShader(const std::string& id)
{
    //grab thje vert and frag seprately based on what shader they want
    //so if they say phong itll filter phong first then look for its vert and frag and combo that
    auto checker = shaderMap.find(id);
    if (checker != shaderMap.end())
    {
        return checker->second;
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
    shaderMap[id] = cshade;
    return cshade;
}
AssetManager::~AssetManager()
{
	for (auto& [key, mesh] : meshMap)
	{
		mesh->OnDestroy();
	}
	meshMap.clear();
	for (auto& [key, mat] : materialMap)
	{
		mat->OnDestroy();
	}
	materialMap.clear();
	for (auto& [key, shader] : shaderMap)
	{
		shader->OnDestroy();
	}
	shaderMap.clear();
    for(int i = 0; i < actorMap.size();i++)
	{
		actorMap[i]->OnDestroy();
	}
	actorMap.clear();

	renderer = nullptr;
}