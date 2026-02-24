#include <glew.h>
#include <iostream>
#include "Debug.h"
#include "Scene3.h"
#include <MMath.h>
#include "Debug.h"
#include "CActor.h"
#include "CShader.h"
#include "CMesh.h"
#include "CMaterial.h"
#include "CTransform.h"
#include "CWorld.h"
#include "CInput.h"
#include "CCamera.h"
#include "CPhysics.h"
#include "CGlobalLight.h"
#include "VulkanRenderer.h"
#include "OpenGLRenderer.h"
#include "AssetManager.h"
#include "FmodController.h"

Scene3::Scene3(EngineContext& context_): 
	Scene(context_) {
	Debug::Info("Created Scene3: ", __FILE__, __LINE__);
}

Scene3::~Scene3() {
}

bool Scene3::OnCreate() {
	int width = 0, height = 0;
	float aspectRatio;
	switch (engineContext.renderer->getRendererType()){
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);
		
		

		SDL_GetWindowSize(vRenderer->getWindow(), &width, &height);
		aspectRatio = static_cast<float>(width) / static_cast<float>(height);	
		
		// step 1 Create the  GLOBAL componetes
		
		Ref<CActor> cam = std::make_shared<CActor>();
		cam->AddComponent<CCamera>(std::make_shared<CCamera>(cam, engineContext.renderer, 70.0f, aspectRatio, 0.25f, 500.0f));
		//cam->AddComponent<CTransform>(std::make_shared<CTransform>(nullptr, Vec3(0, 0, 5), QMath::angleAxisRotation(0.0f, Vec3(0, 1, 0)), Vec3()));
		cam->AddComponent<CPhysics>(std::make_shared<CPhysics>(cam));
		cam->AddComponent<CInput>(std::make_shared<CInput>(cam));
		LightConfig ldata;
		//ldata.diffused = Vec4(0.5f, 0.6f, 0.0f, 0.0f);
		//ldata.specular = Vec4(0.0f, 0.3f, 0.0f, 0.0f);
		//ldata.ambient = Vec4(0.1f, 0.1f, 0.1f, 0.0f);
		ldata.diffused = Vec4(0.5f, 0.6f, 0.8f, 0.0f);
		ldata.specular = Vec4(0.9f, 0.9f, 1.0f, 0.0f);
		ldata.ambient = Vec4(0.1f, 0.1f, 0.2f, 0.0f) * 0.8f;
		
		ldata.orientation =  QMath::angleAxisRotation(-10, Vec3(1, 0, 0));
		ldata.distance = 2.0f;
		float sidelenght = 15.0f;
		OrthConfig config;
		config.xmax = (sidelenght * 0.5f); config.xmin = -(sidelenght * 0.5f); config.ymax = (sidelenght * 0.5f); config.ymin = -(sidelenght * 0.5f);
		config.zmax = sidelenght; config.zmin = 0.25f;
		/*PerspectiveConfig config;
		config.aspectRatio = aspectRatio;
		config.far = 100.0f;
		config.near = 0.5f;
		config.fovy = 45.0f;*/
		cam->AddComponent<CGlobalLight>(std::make_shared<CGlobalLight>(cam, engineContext.renderer, config, ldata));
		if (!cam->OnCreate()) {
			printf(" FAILED TO CREATE CAMERA \n");
		}
		//actorsInScene = engineContext.assetManager->GetActorsInScene();	
		//cam->GetComponent<CPhysics>()->SetPosition(Vec3(0, 0, 5));
		//cam->GetComponent<CPhysics>()->SetRotation(Quaternion());

		
		//vRenderer->DestroyGlobalResources();
		//to get a shadow pass
		// i need rework the main shader
		// adjust the rendering passes in ecs
		// created the memory barrier between renderpasses 
		// Also Todo: Light component and camera component over a actors 
		// need to adjust cshader to use the config pipeline
		//  GET IT WORKING WITHOUT SHADOWS , DONE
		//  MEMORY BARRIER FOR BETWEEN RENDERPASSES , DONE
		//  ADJUST ECS RENDERING TO INCLUDE SHADOW PASS , DONE
		//  SHADER WORK GET SHADOWS , DONE
		//  PROTOTYPE CAMERA AND LIGHT COMPONENTS , DONE
		//  ADD FUCTION TO MAKE A DEFUALT PIPELINE COFIG WITH THE MAIN RENDERPASS , DONE
		//  ADJUST CSHADER USE NEW PIPELINE COFIG , DONE
		// PART TWO UBOS SHOULD UPDATE AND SHOULD ONLY CURRENT FRAME UPDATE , DONE
		// TODO: SKYBOX  - images - pipeline DONE
		// TODO: ADD SKYBOX TO ECS RENDERING DONE
		// TODO: ADD FORWARD Declaration COMPOENT TO VULKAN AND REMOVE INCLUDE DONE 
		// TODO : Compute Boiler work
		// TODO : START ON CLUSETER LIGHTING: PROBLY GOING TO NEED A LIGHT SYSTEM
		//  WHERE componets LIGTHS REGISTERY AND GET ADDE  TO SSBO
		// THIS WILL NEED BOTH COMPUTE AND GRaphic Shaders
		// TODO: FIXING RESIZING THE SCREEN
		
	
		//"./meshes/Mario.obj" , "./textures/mario_mime.png" , "./textures/mario_fire.png"
	/*	 step 1.1 Meshs*/
		Ref<CMesh> mesh = std::make_shared<CMesh>(nullptr, engineContext.renderer, "./meshes/Mario.obj");
	/*	Ref<CMesh> mesh = assetManager.GetMesh("mario");*/
		mesh->OnCreate();	
		Ref<CMesh> mesh1 = std::make_shared<CMesh>(nullptr, engineContext.renderer, "./meshes/Plane.obj");
		/*	Ref<CMesh> mesh = assetManager.GetMesh("mario");*/
		mesh1->OnCreate();

		// step 1.2 shaders

		std::vector<SingleDescriptorSetLayoutInfo> layoutInfo;
		vRenderer->AddToDescriptorLayoutCollection(layoutInfo, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		Ref<CShader> cshade = std::make_shared<CShader>(nullptr, engineContext.renderer,layoutInfo, "shaders/MassPass_2.vert.spv", "shaders/MassPass_2.frag.spv");
		//Ref<CShader> cshade = std::make_shared<CShader>(nullptr, engineContext.renderer, layoutInfo, "shaders/MassPass_2.vert.spv", "shaders/ShadowCheck_2.frag.spv");
		//Ref<CShader> cshade = assetManager.GetShader("phong");
		cshade->OnCreate();
		
		
		//step 1.3 Materials
		std::vector<std::string> filepaths = { "./textures/mario_mime.png" };
		Ref<CMaterial> mat = std::make_shared<CMaterial>(nullptr, engineContext.renderer, filepaths,cshade);
		//Ref<CMaterial> mat = assetManager.GetMat("mario");
		mat->OnCreate();

		filepaths = { "./textures/mario_fire.png" };
		Ref<CMaterial> mat1 = std::make_shared<CMaterial>(nullptr, engineContext.renderer, filepaths, cshade);
		//Ref<CMaterial> mat1 = assetManager.GetMat("mario");
		mat1->OnCreate();

		filepaths = { "./textures/texture_07.png" };

		Ref<CMaterial> mat2 = std::make_shared<CMaterial>(nullptr, engineContext.renderer, filepaths, cshade);
		mat2->OnCreate();


		filepaths = { "./textures/rock.png" };

		Ref<CMaterial> mat3 = std::make_shared<CMaterial>(nullptr, engineContext.renderer, filepaths, cshade);
		mat3->OnCreate();

		// step 2 create actors
		Ref<CActor> act = std::make_shared<CActor>(nullptr);
		Ref<CTransform> t = std::make_shared<CTransform>(nullptr, Vec3(-1, 0, 0), Quaternion(), Vec3(1,1,1));
		act->AddComponent<CTransform>(t);
		act->AddComponent<CMesh>(mesh);
		act->AddComponent<CMaterial>(mat);

		Ref<CActor> act1 = std::make_shared<CActor>(nullptr);
		Ref<CTransform> t1 = std::make_shared<CTransform>(nullptr, Vec3(1.5, -0.5, 0),QMath::angleAxisRotation(90,Vec3(0,1,0)), Vec3(1, 1, 1));
		act1->AddComponent<CTransform>(t1);
		act1->AddComponent<CMesh>(mesh);
		act1->AddComponent<CMaterial>(mat1);

		Ref<CActor> act2 = std::make_shared<CActor>(nullptr);
		Ref<CTransform> t2 = std::make_shared<CTransform>(nullptr, Vec3(0,-1.5,0), QMath::angleAxisRotation(-90, Vec3(1, 0, 0)), Vec3(5, 5, 1));
		act2->AddComponent<CTransform>(t2);
		act2->AddComponent<CMesh>(mesh1);
		act2->AddComponent<CMaterial>(mat2);

		Ref<CActor> WorldActor = std::make_shared<CActor>(nullptr);
		auto wC = std::make_shared<CWorld>(nullptr, engineContext.renderer, TerrainPreset{});

		preset.concatenate = true;
		preset.globalHeightScale = 15.0f;
		preset.base.type = NoiseType::OpenSimplex2;
		preset.base.seed = 3847598;
		preset.base.frequency = 0.009f;
		preset.base.amplitude = 5.0f;
		preset.base.fractal = FractalType::FBm;
		preset.base.fractalOctaves = 2;
		preset.base.gain = 0.3f;
		preset.continentalness.type = NoiseType::Cellular;
		preset.continentalness.frequency = 0.1f;
		preset.continentalness.amplitude = 1.0;
		preset.continentalness.fractal = FractalType::PingPong;
		preset.continentalness.cellType = CellularType::Euclidian;
		preset.continentalness.returnType = ReturnType::Distance; 
		preset.peaksValleys.type = NoiseType::Cubic;
		preset.peaksValleys.cellType = CellularType::Hybrid;
		preset.peaksValleys.returnType = ReturnType::Distance2;
		preset.peaksValleys.fractal = FractalType::PingPong;
		preset.peaksValleys.amplitude = 0.9f;
		preset.peaksValleys.frequency = 9.0f;

		preset.exponent = 1.4f;
		
		
		preset2.base.type = NoiseType::Perlin;
		preset2.base.seed = 421322;	
		preset2.base.frequency = 0.03f;
		preset2.base.amplitude = 5.7f;
		preset2.base.fractal = FractalType::FBm;
		preset2.base.fractalOctaves = 5;
		preset2.base.lacunarity = 0.2f;
		preset2.base.gain = 0.1f;
		preset2.exponent = 2.0f;

		wC->InitializeWorld(&preset);
		WorldActor->AddComponent<CWorld>(wC);
		WorldActor->AddComponent<CMaterial>(mat3);
		WorldActor->OnCreate();
		
		actorsInScene.push_back(WorldActor);
		//actorsInScene.push_back(act2);
		actorsInScene.push_back(act);
		actorsInScene.push_back(act1);
		
		//step 3 Actors being added to the scene.
		actor = act;
		actor1 = act1;
		plane = act2;
		camera = cam;
		shader = cshade;
		world = WorldActor;
		engineContext.fmodController->playsong(0);
		
	}
		break;

	case RendererType::OPENGL:
		break;
	}

	return true;
}

void Scene3::HandleEvents(const SDL_Event& sdlEvent) {
	
		switch (sdlEvent.type) {
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
			printf("size changed %d %d\n", sdlEvent.window.data1, sdlEvent.window.data2);
			float aspectRatio = static_cast<float>(sdlEvent.window.data1) / static_cast<float>(sdlEvent.window.data2);
			///camera->Perspective(45.0f, aspectRatio, 0.5f, 20.0f);
			if (engineContext.renderer->getRendererType() == RendererType::VULKAN)
			{
				dynamic_cast<VulkanRenderer*>(engineContext.renderer)->RecreateSwapChain();
			}
			break;
		}
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
		{
			if (sdlEvent.type == SDL_EVENT_KEY_DOWN) {
				// escape stuff
				if (sdlEvent.key.key == SDLK_ESCAPE) {
					mouseLocked = !mouseLocked;
					SDL_SetWindowRelativeMouseMode(dynamic_cast<VulkanRenderer*>(engineContext.renderer)->getWindow(), mouseLocked);
				}
				else if (sdlEvent.key.key == SDLK_9) {
					auto wA = std::dynamic_pointer_cast<CActor>(world);
					auto w = wA->GetComponent<CWorld>();
					w->OnDestroy();
					w->InitializeWorld(&preset);
				}
				else if (sdlEvent.key.key == SDLK_0) {
					auto wA = std::dynamic_pointer_cast<CActor>(world);
					auto w = wA->GetComponent<CWorld>();
					w->OnDestroy();
					w->InitializeWorld(&preset2);

				}
				else if (sdlEvent.key.key == SDLK_B)
				{
					engineContext.fmodController->playsong(AudioState::PAUSE);
				}
				else if (sdlEvent.key.key == SDLK_N)
				{
					engineContext.fmodController->playsong(AudioState::PLAY);
				}
			}
			auto p1 = std::dynamic_pointer_cast<CActor>(camera);
			auto playerController = p1->GetComponent<CInput>();
			if (playerController) {
				playerController->HandleKeyboardInput(sdlEvent);
			}
			break;
		}

		case SDL_EVENT_MOUSE_MOTION:
		{
			auto p1 = std::dynamic_pointer_cast<CActor>(camera);
			auto playerController = p1->GetComponent<CInput>();
			if (playerController) {
				playerController->HandleMouseMotion(sdlEvent);
			}
			break;
		}

		}
	
}
void Scene3::Update(const float deltaTime) {
	auto player = std::dynamic_pointer_cast<CActor>(camera);
	if (player) {
		auto playerController =player->GetComponent<CInput>();
		auto phys  =player->GetComponent<CPhysics>();
		if (playerController) {
			playerController->UpdateInput(deltaTime);
			phys->Update(deltaTime);
		}
	}
}

void Scene3::Render() const {
	switch (engineContext.renderer->getRendererType()) {

	case RendererType::VULKAN:
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);

		
		{
		/*	std::vector<Ref<Component>> drawlist;
			actorsInScene.push_back(plane);*/
			/*drawlist.push_back(actor);
			drawlist.push_back(actor1);
			drawlist.push_back(plane);
			drawlist.push_back(World);*/
			vRenderer->RenderECS(actorsInScene);// Context obejct
		}
		break;

	case RendererType::OPENGL:
		OpenGLRenderer* glRenderer;
		glRenderer = dynamic_cast<OpenGLRenderer*>(engineContext.renderer);
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


void Scene3::OnDestroy() {
	VulkanRenderer* vRenderer;
	vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);
	if(vRenderer){
		vkDeviceWaitIdle(vRenderer->getDevice());				
		
		std::dynamic_pointer_cast<CShader>(shader)->OnDestroy();
	
		vRenderer->DestroyUBO(lightsUBO);
		
		engineContext.fmodController->playsong(0);
		camera->OnDestroy();
		actor->OnDestroy();
		world->OnDestroy();
		actor1->OnDestroy();
		plane->OnDestroy();
		
		
		}

	
}
