#include "AssetManager.h"
#include "CActor.h"
#include "CMesh.h"
#include "CMaterial.h"

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

Ref<CMesh> AssetManager::GetMesh(const std::string& id)
{
    auto checker = meshMap.find(id);
    if (checker != meshMap.end())
    {
        return checker->second;
    }

    if (!jsonLoader.contains("Shaders") || !jsonLoader["Shaders"].contains(id))
    {
        std::cout << "json does not contain Shader" << id << "\n";
        return nullptr;
    }

    std::pair<std::string, std::string> shaderPaths;
    shaderPaths.first = jsonLoader["Shaders"][id]["frag"].get<std::string>();
    shaderPaths.second = jsonLoader["Shaders"][id]["vert"].get<std::string>();
    //Ref<CMesh> shader = std::make_shared<CShader>(nullptr,renderer, nullptr ,shaderPaths.second,shaderPaths.first);
    //shaderMap[id] = shader;
    return nullptr; //shader;
}

Ref<CMaterial> AssetManager::GetMat(const std::string& id)
{
    return Ref<CMaterial>();
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

    if (!jsonLoader.contains("Meshes") || !jsonLoader["Meshes"].contains(id))
    {
        std::cout << "json does not contain Mesh" << id << "\n";
        return nullptr;
    }

    return Ref<CShader>();
}

