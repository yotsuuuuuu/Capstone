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
#include "CSkyBox.h"
#include "CTransform.h"
#include "CCapsuleCollider.h"
#include "CWorld.h"
#include "CInput.h"
#include "CCamera.h"
#include "CPhysics.h"
#include "CGlobalLight.h"
#include "VulkanRenderer.h"
#include "CustomSDLEvents.h"
#include "SYS_Light.h"
#include "OpenGLRenderer.h"
#include "AssetManager.h"
#include "FmodController.h"
#include "PhysicsManager.h"

Scene3::Scene3(EngineContext& context_): 
	Scene(context_) {
	Debug::Info("Created Scene3: ", __FILE__, __LINE__);
}

Scene3::~Scene3() {
}

bool Scene3::OnCreate() {
	
	switch (engineContext.renderer->getRendererType()){
	case RendererType::VULKAN:
	{
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);		


		auto& actorsInScene = engineContext.assetManager->GetActorsInScene();


		//step 1.3 Materials

		std::vector<std::string> filepaths = { "./textures/DefaultTexture.png" };

		Ref<CMaterial> mat3 = std::make_shared<CMaterial>(nullptr, engineContext.renderer, filepaths, engineContext.assetManager->GetShader("Terrain"));
		mat3->OnCreate();

		// step 2 create actors
		

		Ref<CActor> WorldActor = std::make_shared<CActor>(nullptr);
		auto wC = std::make_shared<CWorld>(nullptr, engineContext, TerrainPreset{});
	

		//wC->InitializeWorld(1);
		WorldActor->AddComponent<CWorld>(wC);
		WorldActor->AddComponent<CMaterial>(mat3);
		WorldActor->OnCreate();
		wC->InitializeWorld(0);
		

		//actorsInScene.push_back(engineContext.assetManager->GetCamera());

		//step 3 Actors being added to the scene.
		camera = engineContext.assetManager->GetCamera();	
		world = WorldActor;
		engineContext.physicsManager->Set(actorsInScene, camera, world);
		engineContext.physicsManager->OnCreate();	
		

		// intial player spawn
		auto cameraActor = std::dynamic_pointer_cast<CActor>(camera);
		Vec3 spawn = wC->GetPlayerSpawn();
		float halfCylinder = cameraActor->GetComponent<CCapsuleCollider>()->GetHalfCylinder();
		spawn.y += halfCylinder * 2.0f;
		cameraActor->GetComponent<CPhysics>()->SetPosition(spawn);
		cameraActor->GetComponent<CPhysics>()->SetVelocity(Vec3(0.0f, 0.0f, 0.0f));

		actorsInScene.push_back(WorldActor);


	}
		break;

	case RendererType::OPENGL:
		break;
	}

	return true;
}

void Scene3::HandleEvents(const SDL_Event& sdlEvent) {
	
	auto p1 = std::dynamic_pointer_cast<CActor>(camera);
	auto playerController = p1->GetComponent<CInput>();
	if (playerController) {
		playerController->HandleKeyboardInput(sdlEvent);
		playerController->HandleMouseMotion(sdlEvent);
	}
	if (sdlEvent.type == CustomEvent::SONG_SELECTED_EVENT) {
		int sondId = (int)sdlEvent.user.code;
		engineContext.assetManager->clearActorsInScene();
		
		//Ref<CMaterial> mat3 = std::make_shared<CMaterial>(nullptr, engineContext.renderer, "./textures/rock.png", engineContext.assetManager->GetShader("Terrain"));
		//mat3->OnCreate();

		Ref<CActor> wA = std::dynamic_pointer_cast<CActor>(world);
		auto w = wA->GetComponent<CWorld>();
		auto mat = wA->GetComponent<CMaterial>()->OnCreate();
		w->InitializeWorld(sondId);

		auto cameraActor = std::dynamic_pointer_cast<CActor>(camera);

		Vec3 spawn = w->GetPlayerSpawn();
		float halfCylinder = cameraActor->GetComponent<CCapsuleCollider>()->GetHalfCylinder();
		spawn.y += halfCylinder * 2.0f;
		spawn.print();
		
		cameraActor->GetComponent<CPhysics>()->SetPosition(spawn);
		cameraActor->GetComponent<CPhysics>()->SetVelocity(Vec3(0.0f, 0.0f, 0.0f));
		
		auto& sceneActors = engineContext.assetManager->GetActorsInScene();
		sceneActors.push_back(wA);

	}
	else if (sdlEvent.type == CustomEvent::PLAYER_RESET_EVENT) {
		auto wA = std::dynamic_pointer_cast<CActor>(world);
		auto w = wA->GetComponent<CWorld>();
		auto cameraActor = std::dynamic_pointer_cast<CActor>(camera);
		Vec3 spawn = w->GetPlayerSpawn();
		float halfCylinder = cameraActor->GetComponent<CCapsuleCollider>()->GetHalfCylinder();		
		spawn.y += halfCylinder * 2.0f;
		cameraActor->GetComponent<CPhysics>()->SetPosition(spawn);
		cameraActor->GetComponent<CPhysics>()->SetVelocity(Vec3(0.0f, 0.0f, 0.0f));

	}
	switch (sdlEvent.type) {
	case SDL_EVENT_KEY_UP:{
		
		break;
	}	

	}
	
}

void Scene3::FrustumCheck()
{
	auto cam = std::dynamic_pointer_cast<CActor>(camera);
	auto camComp = cam->GetComponent<CCamera>();
	auto worldActor = std::dynamic_pointer_cast<CActor>(world);
	std::vector<MATHEX::Plane> frustum = camComp->GenerateFrustumPlane();
	auto chunksData = worldActor->GetComponent<CWorld>()->GetChunkRenderData();
	for (auto& c : *chunksData) {
		c.isCulled = false;

		for (int i = 0; i < 6; i++) {
	
			const MATHEX::Plane& p = frustum[i];

			Vec3 pVertex;

			pVertex.x = (p.n.x > 0) ? c.aabb.max.x : c.aabb.min.x;
			pVertex.y = (p.n.y > 0) ? c.aabb.max.y : c.aabb.min.y;
			pVertex.z = (p.n.z > 0) ? c.aabb.max.z : c.aabb.min.z;

			float dot = p.n.x * pVertex.x + 
						p.n.y * pVertex.y + 
						p.n.z * pVertex.z;

			if (dot < -p.d) { 
				c.isCulled = true;
				//std::cout << "culled chunk at pos: " << std::endl;
				break;
			}

		}
	}

}



void Scene3::Update(const float deltaTime) {

	engineContext.physicsManager->Update(deltaTime);
	auto Skybox = std::dynamic_pointer_cast<CActor>(camera)->GetComponent<CSkyBox>();
	if (Skybox) {
		Skybox->AudioReact(engineContext);
	}
	// TODO: move to camera
	FrustumCheck();
	
	// world collision checking
	// through a system maybe?
	//
	// iterate through the chunk render data
	// keep track 
	// int mapX, mapY; (start at 0)
	// iterate through and when 
	// mapX < WORLD_SIZE
	// mapX = 0;
	// mapY++
	// when mapY < WORLD_SIZE && mapX < WORLD_SIZE -> return;
	// mapX and mapY can be used to grab the correct chunk from chunkMap
	// chunkMap.find(Vec2(mapX, mapY));
	// chunkMap holds indices and vertices on CPU so can be used to check collision with player
	//  
	//
	
}

void Scene3::Render() const {
	switch (engineContext.renderer->getRendererType()) {

	case RendererType::VULKAN:
		VulkanRenderer* vRenderer;
		vRenderer = dynamic_cast<VulkanRenderer*>(engineContext.renderer);

		
		{		
			vRenderer->RenderECS(engineContext, engineContext.assetManager->GetActorsInScene());
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
		
		
	}

	
}
