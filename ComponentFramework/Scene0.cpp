#include <glew.h>
#include <iostream>
#include "Debug.h"
#include "Scene0.h"
#include <MMath.h>
#include "Debug.h"
#include "CActor.h"
#include "CShader.h"
#include "CMesh.h"
#include "CMaterial.h"
#include "CTransform.h"
#include "CCamera.h"
#include "CCameraActor.h"
#include "VulkanRenderer.h"
#include "OpenGLRenderer.h"
#include "AssetManager.h"

Scene0::Scene0(Renderer *renderer_): 
	Scene(nullptr),renderer(renderer_) {
	Debug::Info("Created Scene0: ", __FILE__, __LINE__);
}

Scene0::~Scene0() {
}

bool Scene0::OnCreate() {
	int width = 0, height = 0;
	float aspectRatio;
	AssetManager assetManager(static_cast<VulkanRenderer*>(renderer));
	switch (renderer->getRendererType()){
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
		
		

		SDL_GetWindowSize(vRenderer->getWindow(), &width, &height);
		aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		//camera.projectionMatrix = MMath::perspective(45.0f, aspectRatio, 0.5f, 100.0f);
		//camera.projectionMatrix[5] *= -1.0f;
		//camera.viewMatrix = MMath::translate(0.0f, 0.0f, -5.0f);
		
		// step 1 Create the  GLOBAL componetes
		/*Ref<CCameraActor> cam = std::make_shared<CCameraActor>(nullptr, renderer);
		cam->AddComponent<CTransform>(std::make_shared<CTransform>(nullptr, Vec3(-6, 0, 4), QMath::angleAxisRotation(-45.0f,Vec3(0,1,0)), Vec3()));
		cam->UpdateProjectionMatrix(45.0f, aspectRatio, 0.5f, 100.0f);
		cam->UpdateViewMatrix();
		cam->OnCreate();
		cam->UpdateUBO(0);*/
		Ref<CActor> cam = std::make_shared<CActor>();
		cam->AddComponent<CCamera>(std::make_shared<CCamera>(cam, renderer, 45.0f, aspectRatio, 0.5f, 100.0f));
		cam->AddComponent<CTransform>(std::make_shared<CTransform>(nullptr, Vec3(-6, 0, 4), QMath::angleAxisRotation(-45.0f, Vec3(0, 1, 0)), Vec3()));

		if (!cam->OnCreate()) {
			printf(" FAILED TO CREATE CAMERA \n");
		}
		vRenderer->CreateGlobalRources(cam->GetComponent<CCamera>()->GetCameraUBO());
		//vRenderer->DestroyGlobalResources();
		//to get a shadow pass
		// i need rework the main shader
		// adjust the rendering passes in ecs
		// created the memory barrier between renderpasses 
		// Also Todo: Light component and camera component over a actors 
		// need to adjust cshader to use the config pipeline
		// TODO: GET IT WORKING WITHOUT SHADOWS , DONE
		// TODO: MEMORY BARRIER FOR BETWEEN RENDERPASSES , DONE
		// TODO: ADJUST ECS RENDERING TO INCLUDE SHADOW PASS , DONE
		// TODO: SHADER WORK GET SHADOWS , DONE
		// TODO: ADJUST CSHADER USE NEW PIPELINE COFIG
		// TODO: PROTOTYPE CAMERA AND LIGHT COMPONENTS , PART TWO UBOS SHOULD UPDATE AND SHOULD ONLY UPDATE
		// CURRENT FRAMES UBO NOT ALL UBOS AT THE SAME TIME
	
		
	/*	lightsUBO = vRenderer->CreateUniformBuffers<LightsData>();
		lights.diffuse[0] = Vec4(0.5f, 0.6f, 0.0f, 0.0f);
		lights.specular[0] = Vec4(0.0f, 0.3f, 0.0f, 0.0f);
		lights.ambient = Vec4(0.1f, 0.1f, 0.1f, 0.0f);
		lights.numLights = 1;
		lights.pos[0] = Vec4(-4.0f, 0.0f, -5.0f, 0.0f);
		vRenderer->UpdateUniformBuffers<LightsData>(lights, lightsUBO);
		std::vector<SingleDescriptorSetLayoutInfo> layoutGlobal;
		vRenderer->AddToDescriptorLayoutCollection(layoutGlobal, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);
		vRenderer->AddToDescriptorLayoutCollection(layoutGlobal, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		std::vector<DescriptorWriteInfo> writeGlobal;
		vRenderer->AddToDescrisptorLayoutWrite(writeGlobal, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DescriptorWriteInfo::Destype::UBO, VK_SHADER_STAGE_VERTEX_BIT, 1,cam->GetCameraUBO());
		vRenderer->AddToDescrisptorLayoutWrite(writeGlobal, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DescriptorWriteInfo::Destype::UBO, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1,lightsUBO);
		vRenderer->CreateGlobalDescriptionSet(layoutGlobal, writeGlobal);*/

		//"./meshes/Mario.obj" , "./textures/mario_mime.png" , "./textures/mario_fire.png"
	/*	 step 1.1 Meshs*/
		assetManager.LoadAsset("./test.json");
		Ref<CMesh> mesh = std::make_shared<CMesh>(nullptr, renderer, "./meshes/Mario.obj");
	/*	Ref<CMesh> mesh = assetManager.GetMesh("mario");*/
		mesh->OnCreate();	
		Ref<CMesh> mesh1 = std::make_shared<CMesh>(nullptr, renderer, "./meshes/Plane.obj");
		/*	Ref<CMesh> mesh = assetManager.GetMesh("mario");*/
		mesh1->OnCreate();

		// step 1.2 shaders

		std::vector<SingleDescriptorSetLayoutInfo> layoutInfo;
		vRenderer->AddToDescriptorLayoutCollection(layoutInfo, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		Ref<CShader> cshade = std::make_shared<CShader>(nullptr,renderer,layoutInfo, "shaders/MainPass.vert.spv", "shaders/MainPass.frag.spv");		
		//Ref<CShader> cshade = assetManager.GetShader("phong");
		cshade->OnCreate();
		
		//step 1.3 Materials
		std::vector<std::string> filepaths = { "./textures/mario_mime.png" };
		Ref<CMaterial> mat = std::make_shared<CMaterial>(nullptr, renderer, filepaths,cshade);
		//Ref<CMaterial> mat = assetManager.GetMat("mario");
		mat->OnCreate();

		filepaths = { "./textures/mario_fire.png" };
		Ref<CMaterial> mat1 = std::make_shared<CMaterial>(nullptr, renderer, filepaths, cshade);
		//Ref<CMaterial> mat1 = assetManager.GetMat("mario");
		mat1->OnCreate();

		filepaths = { "./textures/checkered_board.png" };
		Ref<CMaterial> mat2 = std::make_shared<CMaterial>(nullptr, renderer, filepaths, cshade);
		mat2->OnCreate();

		// step 2 create actors
		Ref<CActor> act = std::make_shared<CActor>(nullptr);
		Ref<CTransform> t = std::make_shared<CTransform>(nullptr, Vec3(-1, 0, 0), Quaternion(), Vec3(1,1,1));
		act->AddComponent<CTransform>(t);
		act->AddComponent<CMesh>(mesh);
		act->AddComponent<CMaterial>(mat);

		Ref<CActor> act1 = std::make_shared<CActor>(nullptr);
		Ref<CTransform> t1 = std::make_shared<CTransform>(nullptr, Vec3(1, 0, 0),QMath::angleAxisRotation(90,Vec3(0,1,0)), Vec3(1, 1, 1));
		act1->AddComponent<CTransform>(t1);
		act1->AddComponent<CMesh>(mesh);
		act1->AddComponent<CMaterial>(mat1);

		Ref<CActor> act2 = std::make_shared<CActor>(nullptr);
		Ref<CTransform> t2 = std::make_shared<CTransform>(nullptr, Vec3(0, 0,-10), QMath::angleAxisRotation(-25, Vec3(1, 0, 0)), Vec3(3, 3, 1));
		act2->AddComponent<CTransform>(t2);
		act2->AddComponent<CMesh>(mesh1);
		act2->AddComponent<CMaterial>(mat2);
		
		//step 3 Actors being added to the scene.
		actor = act;
		actor1 = act1;
		plane = act2;
		camera = cam;
		shader = cshade;
		
	}
		break;

	case RendererType::OPENGL:
		break;
	}

	return true;
}

void Scene0::HandleEvents(const SDL_Event& sdlEvent) {
	
		switch (sdlEvent.type) {
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			printf("size changed %d %d\n", sdlEvent.window.data1, sdlEvent.window.data2);
			float aspectRatio = static_cast<float>(sdlEvent.window.data1) / static_cast<float>(sdlEvent.window.data2);
			///camera->Perspective(45.0f, aspectRatio, 0.5f, 20.0f);
			if(renderer->getRendererType() == RendererType::VULKAN){
				dynamic_cast<VulkanRenderer*>(renderer)->RecreateSwapChain();
			}
			break;
		}
	
}
void Scene0::Update(const float deltaTime) {
	
}

void Scene0::Render() const {
	switch (renderer->getRendererType()) {

	case RendererType::VULKAN:
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(renderer);

		
		{
			std::vector<Ref<Component>> drawlist;
			drawlist.push_back(actor);
			drawlist.push_back(actor1);
			drawlist.push_back(plane);
			vRenderer->RenderECS(drawlist);
		}
		break;

	case RendererType::OPENGL:
		OpenGLRenderer* glRenderer;
		glRenderer = dynamic_cast<OpenGLRenderer*>(renderer);
		/// Clear the screen
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		/// Draw your scene here
		glUseProgram(0);
		
		break;
	}
}


void Scene0::OnDestroy() {
	VulkanRenderer* vRenderer;
	vRenderer = dynamic_cast<VulkanRenderer*>(renderer);
	if(vRenderer){
		vkDeviceWaitIdle(vRenderer->getDevice());
		// the life time of the cmd buffers is bound to the cmd pool
		// and i don't think the life time should be tied to the scene
		// so commeted out and moved the destruction of primary cmd and the pool
		// to the OnDestroy of the VulkanRenderer
		// On the Same note: in recreate swapchains
		// I removed the creation on of new cmd buffers
		// 
		//vRenderer->DestroyCommandBuffers(); 

		
		//vRenderer->DestroyGlobalDescriptionSet(); // note eventaully need to get moved out of the scene.
		vRenderer->DestroyGlobalResources();
		std::dynamic_pointer_cast<CShader>(shader)->OnDestroy();
		vRenderer->DestroyUBO(lightsUBO);
		
		camera->OnDestroy();
		actor->OnDestroy();
		actor1->OnDestroy();
		plane->OnDestroy();
		
		
		}

	
}
