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

    if (!jsonLoader.contains("Meshes") || !jsonLoader["Meshes"].contains(id))
    {
        std::cout << "json does not contain Mesh" << id << "\n";
        return nullptr;
    }

    std::string filepath = jsonLoader["Meshes"][id].get<std::string>();
    Ref<CMesh> mesh = std::make_shared<CMesh>(nullptr,renderer,filepath);
    meshMap[id] = mesh;
    return mesh;
}

Ref<CMaterial> AssetManager::GetMat(const std::string& id)
{
    return Ref<CMaterial>();
}

Ref<CShader> AssetManager::GetShader(const std::string& id)
{
    //grab thje vert and frag seprately based on what shader they want
    //so if they say phong itll filter phong first then look for its vert and frag and combo that
    return Ref<CShader>();
}

