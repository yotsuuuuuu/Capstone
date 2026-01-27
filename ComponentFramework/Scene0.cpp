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
		
		lightsUBO = vRenderer->CreateUniformBuffers<LightsData>();
		lights.diffuse[0] = Vec4(0.5, 0.6, 0.0, 0.0);
		lights.specular[0] = Vec4(0.0, 0.3, 0.0, 0.0);
		lights.ambient = Vec4(0.1, 0.1, 0.1, 0.0);
		lights.numLights = 1;
		lights.pos[0] = Vec4(-4.0f, 0.0f, -5.0f, 0.0f);
		vRenderer->UpdateUniformBuffer<LightsData>(lights, lightsUBO);
		

		SDL_GetWindowSize(vRenderer->getWindow(), &width, &height);
		aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		//camera.projectionMatrix = MMath::perspective(45.0f, aspectRatio, 0.5f, 100.0f);
		//camera.projectionMatrix[5] *= -1.0f;
		//camera.viewMatrix = MMath::translate(0.0f, 0.0f, -5.0f);
		
		// step 1 Create the componetes
		Ref<CCameraActor> cam = std::make_shared<CCameraActor>(nullptr, renderer);
		cam->AddComponent<CTransform>(std::make_shared<CTransform>(nullptr, Vec3(0, 0, 5), Quaternion(),Vec3()));
		cam->UpdateProjectionMatrix(45.0f, aspectRatio, 0.5f, 100.0f);
		cam->UpdateViewMatrix();
		cam->OnCreate();
		cam->UpdateUBO(0);
		//"./meshes/Mario.obj" , "./textures/mario_mime.png" , "./textures/mario_fire.png"
		// step 1.1 meshs
		Ref<CMesh> mesh = std::make_shared<CMesh>(nullptr, renderer,"./meshes/Mario.obj" );
		//assetManager.LoadAsset("test.json");
		//Ref<CMesh> mesh = assetManager.GetMesh("mario");
		mesh->OnCreate();		
		// step 1.2 shaders
		std::vector<SingleDescriptorSetLayoutInfo> layoutInfo;
		vRenderer->AddToDescrisptorLayoutCollection(layoutInfo, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);
		vRenderer->AddToDescrisptorLayoutCollection(layoutInfo, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		vRenderer->AddToDescrisptorLayoutCollection(layoutInfo, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);	
		desSetInfo.descriptorSetLayout = vRenderer->CreateDescriptorSetLayout(layoutInfo);

		// step 1.3.1 count  how many materials for which shader to size the pool correctly
		desSetInfo.descriptorPool = vRenderer->CreateDescriptorPool(layoutInfo, 2);
		// SHADERS NEEDS LayoutInfo, paths, and renderer
		Ref<CShader> cshade = std::make_shared<CShader>(nullptr, renderer, desSetInfo.descriptorSetLayout, "shaders/multiPhong.vert.spv", "shaders/multiPhong.frag.spv");
		cshade->OnCreate();
		
		//step 1.3 material
		std::vector<std::pair<std::string, SingleDescriptorSetLayoutInfo>> array; // need this form vicent filled with the write data in the material
		Ref<CMaterial> mat = std::make_shared<CMaterial>(nullptr, renderer, "./textures/mario_mime.png");
		mat->OnCreate();
		mat->SetShader(cshade);
		Ref<CMaterial> mat1 = std::make_shared<CMaterial>(nullptr, renderer, "./textures/mario_fire.png");
		mat1->OnCreate();
		mat1->SetShader(cshade);
		// step 1.3.2 allocate form the pool
		auto set = vRenderer->AllocateDescriptorSets(desSetInfo.descriptorPool, desSetInfo.descriptorSetLayout);
		auto set1 = vRenderer->AllocateDescriptorSets(desSetInfo.descriptorPool, desSetInfo.descriptorSetLayout);
		// step 1.3.4 write to set
		std::vector<DescriptorWriteInfo> write;
		vRenderer->AddToDescrisptorLayoutWrite(write, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, cam->GetCameraUBO());
		vRenderer->AddToDescrisptorLayoutWrite(write, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, lightsUBO);
		auto matrefence = mat->GetTextureSampler(); // getting the handle form the loaded texture to add as a descriptor
		vRenderer->AddToDescrisptorLayoutWrite(write, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, &matrefence);		
		vRenderer->WriteDescriptorSets(set, write);
		// step 1.3.5 set set to Matrial
		mat->SetDescriptorSet(set);
		write.clear();
		// step 1.3.6 do the same for all materials that share the layout
		vRenderer->AddToDescrisptorLayoutWrite(write, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, cam->GetCameraUBO());
		vRenderer->AddToDescrisptorLayoutWrite(write, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, lightsUBO);
		matrefence = mat1->GetTextureSampler();
		vRenderer->AddToDescrisptorLayoutWrite(write, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, &matrefence);		
		vRenderer->WriteDescriptorSets(set1, write);

		mat1->SetDescriptorSet(set1);


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
		
		//step 3 Actors being added to the scene.
		actor = act;
		actor1 = act1;
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
		// notes for me:
		// UBO should be only updated when it is not being worked on by the GPU
		// we should use fences to ensure that
		// thought: maybe we can wait for fence for like fractions of an ms 
		// and see it vulkan can give be a success for the wait or not.
		// apperently we can get the status on a fence using vkGetFenceStatus 
		// but probly best idea to wait for fence for the current frame to be finished before updating UBOs
		// todo : 1 implement function that waits for current frame and return a stuct with context info for that frame
		// 2 use cntx to update UBOs
		// 3 record on the right cmd buffer
		// 4 submit the cmd buffer for that frame only

		
		vRenderer->RecordCommandBuffers(Recording::START);
		//actor 1
		{
			auto a = std::dynamic_pointer_cast<CActor>(actor);
			auto mesh = a->GetComponent<CMesh>();
			auto mat = a->GetComponent<CMaterial>();
			auto info = mat->GetPipelineInfo();
			vRenderer->BindPipeline(info.pipeline);		
			vRenderer->BindDescriptorSet(info.pipelineLayout, mat->GetDescriptorSet());
			vRenderer->SetPushConstant(info, a->GetModelMatrix());
			vRenderer->BindMesh(mesh->GetMesh());
			vRenderer->DrawIndexed(mesh->GetMesh());
		}

		//actor 2
		{
			auto a = std::dynamic_pointer_cast<CActor>(actor1);
			auto mesh = a->GetComponent<CMesh>();
			auto mat = a->GetComponent<CMaterial>();
			auto info = mat->GetPipelineInfo();
			vRenderer->BindPipeline(info.pipeline);
			vRenderer->BindDescriptorSet(info.pipelineLayout, mat->GetDescriptorSet());
			vRenderer->SetPushConstant(info, a->GetModelMatrix());
			vRenderer->BindMesh(mesh->GetMesh());
			vRenderer->DrawIndexed(mesh->GetMesh());
		}
	

		vRenderer->RecordCommandBuffers(Recording::STOP);
		vRenderer->Render();
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

		
		vRenderer->DestroyDescriptorSet(desSetInfo);
		std::dynamic_pointer_cast<CShader>(shader)->OnDestroy();
		vRenderer->DestroyUBO(lightsUBO);
		std::dynamic_pointer_cast<CCameraActor>(camera)->OnDestroy();

		actor->OnDestroy();
		actor1->OnDestroy();
		
		
		}

	
}
