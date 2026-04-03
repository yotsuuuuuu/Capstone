#include "AssetManager.h"
#include "CActor.h"
#include "CMesh.h"
#include "CMaterial.h"
#include "CShader.h"
#include "CTransform.h"
#include "CCamera.h"
#include "CPhysics.h"
#include "CCapsuleCollider.h"
#include "CInput.h"
#include "CSkyBox.h"
#include "CGlobalLight.h"
#include "CLight.h"
#include "Debug.h"

using json = nlohmann::json;

bool AssetManager::LoadCamera(const std::string& filepath_)
{
      std::ifstream file(filepath_);

    if (!file.is_open())
    {
        std::cerr << "Failed to open asset database: " << filepath_ << "\n";
        return false;
    }

        jsonLoader = nlohmann::json::parse(file);

    if (!jsonLoader.contains("Camera"))
    {
        std::cout << "json does not contain an Camera section " << "\n";
        return false;
    }

	VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);

    camera = std::make_shared<CActor>(nullptr);
    auto pos = jsonLoader["Camera"]["position"].get<std::vector<float>>();
    auto rot = jsonLoader["Camera"]["rotation"].get<std::vector<float>>();

    float fov = jsonLoader["Camera"]["fov"].get<float>();
    float nearClip = jsonLoader["Camera"]["nearClip"].get<float>();
    float farClip = jsonLoader["Camera"]["farClip"].get<float>();
    
    int height = 0, width = 0;
    float aspectRatio;
	SDL_GetWindowSize(renderer->getWindow(), &width, &height);
	aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    camera->AddComponent<CCamera>(std::make_shared<CCamera>(camera, renderer, fov, aspectRatio, nearClip, farClip));
    camera->AddComponent<CPhysics>(std::make_shared<CPhysics>(camera));
    camera->AddComponent<CInput>(std::make_shared<CInput>(camera));
    camera->AddComponent<CCapsuleCollider>(std::make_shared<CCapsuleCollider>(camera));


    if (!jsonLoader.contains("globalLight"))
    {
        std::cout << "json does not contain an globalLight section " << "\n";
        return false;
    }


    //light creation will go here
    LightConfig ldata;
    auto& lightData = jsonLoader["globalLight"];
    ldata.diffused = Vec4(lightData["diffused"][0].get<float>(), lightData["diffused"][1].get<float>(), lightData["diffused"][2].get<float>(), lightData["diffused"][3].get<float>());        
    ldata.specular = Vec4(lightData["specular"][0].get<float>(), lightData["specular"][1].get<float>(), lightData["specular"][2].get<float>(), lightData["specular"][3].get<float>());
    ldata.ambient = Vec4(lightData["ambient"][0].get<float>(), lightData["ambient"][1].get<float>(), lightData["ambient"][2].get<float>(), lightData["ambient"][3].get<float>());		
    ldata.orientation = QMath::angleAxisRotation(lightData["orientation"][0].get<float>(), Vec3(lightData["orientation"][1].get<float>(), lightData["orientation"][2].get<float>(), lightData["orientation"][3].get<float>()));
    ldata.distance = lightData["distance"].get<float>();
    OrthConfig config;
    float sidelength = lightData["orthosidelength"].get<float>();
    config.xmax = (sidelength * 0.5f); config.xmin = -(sidelength * 0.5f); config.ymax = (sidelength * 0.5f); config.ymin = -(sidelength * 0.5f);
    config.zmax = sidelength; config.zmin = 0.25f;

    camera->AddComponent<CGlobalLight>(std::make_shared<CGlobalLight>(camera, renderer, config, ldata));
    


    if (!camera->OnCreate())
    {
        std::cout << "Failed to create camera" << "\n";
        return false;
    }

    file.close();

    return true;
}

bool AssetManager::LoadAsset(const std::string& filepath_)
{
    std::ifstream file(filepath_);

    if (!file.is_open())
    {
        std::cerr << "Failed to open asset database: " << filepath_ << "\n";
        return false;
    }

        jsonLoader = nlohmann::json::parse(file);

    if (!jsonLoader.contains("Camera"))
    {
        std::cout << "json does not contain an Camera section " << "\n";
        return false;
    }

	VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);


    if (!jsonLoader.contains("Meshes"))
    {
        std::cout << "json does not contain meshes" << "\n";
        return false;
    }


	for (auto& [meshId, meshPath] : jsonLoader["Meshes"].items())
	{
        Ref<CMesh> mesh = std::make_shared<CMesh>(nullptr, renderer, meshPath.get<std::string>());

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
    //note still only does one binding
    for (auto& [shaderId, shaderData] : jsonLoader["Shaders"].items())
    {
        std::vector<SingleDescriptorSetLayoutInfo> layoutInfo;
        std::pair<std::string, std::string> shaderPaths;
        shaderPaths.first = shaderData["frag"].get<std::string>();
        shaderPaths.second = shaderData["vert"].get<std::string>();
        //std::nullopt; 
        std::vector<int> shaderBinding = shaderData["binding"].get<std::vector<int>>();
        //lop from here change stuff to vector
        
        if (shaderBinding.size() != shaderData["type"].size())
            std::cout << "not enough types for amount of bindings";
        if(shaderBinding.size() != shaderData["stage"].size())
			std::cout << "not enough stages for amount of bindings";

        for (int i = 0; i < shaderBinding.size(); i++)
        {
        
        int shaderType = shaderData["type"][i].get<int>();
        int shaderStage = shaderData["stage"][i].get<int>();
        //loop for binding point
        renderer->AddToDescriptorLayoutCollection(layoutInfo, i, static_cast<VkDescriptorType>(shaderType), static_cast<VkShaderStageFlagBits>(shaderStage), 1);
        }
        /*	Ref<CShader> cshade = std::make_shared<CShader>(nullptr, renderer, layoutInfo, shaderPaths.second, shaderPaths.first,config);*/

        PipeLineConfig config = dynamic_cast<VulkanRenderer*>(engineContext.renderer)->GetMainPassPipeLineConfig();
   
        if (shaderData.contains("config"))
        {
            auto& configPath = shaderData["config"];
            //working on translator so its not just a bunch of enums
            if(configPath.contains("topology"))
			configPath.at("topology").get_to(config.topology);
            if(configPath.contains("polygonMode"))
			configPath.at("polygonMode").get_to(config.polygonMode);
            if(configPath.contains("cullMode"))
			configPath.at("cullMode").get_to(config.cullMode);
            if(configPath.contains("blendMode"))
            configPath.at("blendMode").get_to(config.blendMode);
            if(configPath.contains("depthCompareOp"))
            configPath.at("depthCompareOp").get_to(config.depthCompareOp);
            
            //vk_true false 
            if (configPath.contains("depthBias"))
            configPath.at("depthBias").get_to(config.depthBias);
            if (configPath.contains("depthTestEnable"))
            configPath.at("depthTestEnable").get_to(config.depthTestEnable);
            if (configPath.contains("depthWriteEnable"))
            configPath.at("depthWriteEnable").get_to(config.depthWriteEnable);

            //floats
            if (configPath.contains("depthBiasClamp"))
            configPath.at("depthBiasClamp").get_to(config.depthBiasClamp);
            if (configPath.contains("depthBiasSlopeFactor"))
            configPath.at("depthBiasSlopeFactor").get_to(config.depthBiasSlopeFactor);
            if (configPath.contains("depthBiasConstantFactor"))
            configPath.at("depthBiasConstantFactor").get_to(config.depthBiasConstantFactor);

            //normal bool
            if (configPath.contains("dynamicViewport"))
            configPath.at("dynamicViewport").get_to(config.dynamicViewport);


            Ref<CShader> cshade = std::make_shared<CShader>(nullptr, renderer, layoutInfo, shaderPaths.second, shaderPaths.first, config);
            if (!cshade->OnCreate())
            {
                std::cout << "Failed to create shader :" << shaderId << "\n";
            }
            assetMap[shaderId] = cshade;
        }
        else
        {
            Ref<CShader> cshade = std::make_shared<CShader>(nullptr, renderer, layoutInfo, shaderPaths.second, shaderPaths.first);
            if (!cshade->OnCreate())
            {
                std::cout << "Failed to create shader :" << shaderId << "\n";
            }
            assetMap[shaderId] = cshade;
        }
        /*  if (!cshade->OnCreate())
          {
              std::cout << "Failed to create shader :" << shaderId << "\n";
          }
          assetMap[shaderId] = cshade;
      }*/

    }

    if (!jsonLoader.contains("Material")) 
    {
        std::cout << "json does not contain material" << "\n";
        return false;
    }

    for (auto& [matId, matData] : jsonLoader["Material"].items())
    {

        std::vector<std::string> texName = matData["texture"].get<std::vector<std::string>>();
        std::vector<std::string> propertexs;
        for (int i = 0; i < texName.size(); i++)
        {

            propertexs.push_back(jsonLoader["Textures"][texName[i]].get<std::string>());
        }
        Ref<CMaterial> mat1 = std::make_shared<CMaterial>(nullptr, renderer, propertexs, assetMapGet<CShader>(matData["shader"]));
        assetMap[matId] = mat1;

    }

	file.close();
    return true;
}


std::vector<Ref<CActor>> AssetManager::CreateActor(const std::string& actorId, int amount_)
{
    std::vector<Ref<CActor>> actorsMade;
    if (!jsonLoader.contains("Actor"))
    {
        std::cout << "json does not contain an actor section " << "\n";

    }
   
    if (!jsonLoader["Actor"].contains(actorId))
    {
        std::cout << "json does not contain actor with id: " << actorId << "\n";
    }

    auto& actorData = jsonLoader["Actor"][actorId];

    std::string name;
 

    for (int i = 0; i < amount_; i++)
    {


        Ref<CActor> act = std::make_shared<CActor>(nullptr);
        Ref<CTransform> trans = std::make_shared<CTransform>();
       
        if (actorData.contains("Mesh"))
        {
			name = actorData["Mesh"].get<std::string>();
			if (assetMapGet<CMesh>(name))
			{
				act->AddComponent<CMesh>(assetMapGet<CMesh>(name));
			}
            else
            {
				std::string a = "Mesh with id: " + name + " not found for actor. default mesh given " + actorId + "\n";
				Debug::Error(a,__FILE__,__LINE__);
				act->AddComponent<CMesh>(assetMapGet<CMesh>(jsonLoader["Actor"]["DebugSphere"]["Mesh"].get<std::string>()));
 
            }
        }

        if (actorData.contains("Mat"))
        {
			name = actorData["Mat"].get<std::string>();
            if (assetMapGet<CMaterial>(name))
            {
				act->AddComponent<CMaterial>(assetMapGet<CMaterial>(name));
            }
            else
            {
				std::cout << "Material with id: " << name << " not found for actor. default material given " << actorId << "\n";
				act->AddComponent<CMaterial>(assetMapGet<CMaterial>(jsonLoader["Actor"]["DebugSphere"]["Mat"].get<std::string>()));
            }
        }

        act->AddComponent<CTransform>(trans);
     
        float radius = 0.0f;
		float intensity = 0.0f; 
		Vec3 colour = Vec3(0.0,0.0,0.0);
		if (actorData.contains("radius"))
		{
			radius = actorData["radius"].get<float>();
		}
        
        if (actorData.contains("intensity"))
        {
			intensity = actorData["intensity"].get<float>();
        }

        if (actorData.contains("intensity"))
        {
           
		   colour = Vec3(actorData["color"][0].get<float>(), actorData["color"][1].get<float>(), actorData["color"][2].get<float>());
           Ref<CLight>light;
           if (actorData.contains("audioId"))
           {
               int audioId = -1;
               actorData.at("audioId").get_to(audioId);
               light = std::make_shared<CLight>(act, engineContext.lightSys, radius, intensity, colour,audioId);
           }
           else
           {
               light = std::make_shared<CLight>(act, engineContext.lightSys, radius, intensity, colour);
           }
		   act->AddComponent<CLight>(light);
        }
        
            
        if (!act->OnCreate())
        {
            std::cout << "Failed to create actor :" << actorId << "\n";
        }

      actorMap.push_back(act);
	  actorsMade.push_back(act);
      //change it to return what was requested
    }

    return actorsMade;
}

std::vector<std::shared_ptr<Component>>& AssetManager::GetActorsInScene()
{
    return actorMap;
}

void AssetManager::clearActorsInScene()
{
    vkDeviceWaitIdle(dynamic_cast<VulkanRenderer*>(engineContext.renderer)->getDevice());
    for (size_t i = 0; i < actorMap.size();i++)
    {
        actorMap[i]->OnDestroy();
    }
    actorMap.clear();
    actorMap.resize(0);
}

Ref<Component> AssetManager::GetCamera()
{
    return camera;
}

Ref<CMesh> AssetManager::GetMesh(const std::string& id)
{
    auto checker = assetMapGet<CMesh>(id);
    VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);
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
        //checker->OnCreate();
        return checker;
    }

    if (!jsonLoader.contains("Material") || !jsonLoader["Material"].contains(id))
    {
        std::cout << "json does not contain meshes" << id << "\n";
        return nullptr;
    }
   
    VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);
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
    
    VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);
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

	camera->OnDestroy();
    camera = nullptr;
}

void AssetManager::RecreatedPipelines()
{
    for (const auto& Comp : assetMap) {
        if (auto shader = std::dynamic_pointer_cast<CShader>(Comp.second)) {
            shader->RecreatePipeLine();
        }
    }
}


void AssetManager::ScreenResizeCameraEvent(float aspectRatio)
{
    auto camComp = camera->GetComponent<CCamera>();
    auto camdata = camComp->GetProjMatrixValues();

    camComp->UpdateProjectionMatrix(camdata[0], aspectRatio, camdata[2], camdata[3]);
    if (auto skybox = camera->GetComponent<CSkyBox>()) {
        skybox->RecreatePipeline();
    }
}


