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
		
		//auto lights = engineContext.assetManager->CreateActor("lightMagenta", 200);
		//auto LightMat = engineContext.assetManager->GetMat("SimpleLightMat");
		//auto LightMesh = engineContext.assetManager->GetMesh("IcoMesh");

		//if (!LightMat->OnCreate()) {
		//	Debug::Warning("LightMat Failed ", __FILE__, __LINE__);
		//	return false;
		//}
		//if (!LightMesh->OnCreate()) {
		//	Debug::Warning("LightMesh Failed ", __FILE__, __LINE__);
		//	return false;
		//}

		//engineContext.assetManager->CreateActor("mario",1);


		auto& actorsInScene = engineContext.assetManager->GetActorsInScene();

		// Spacing = diameter (radius 1 = diameter 2) so lights dont overlap
		//const float spacing = 20.0f;
		//int gridSize = static_cast<int>(std::ceil(std::sqrt(lights.size())));
		
		//static const Vec3 testColors[] = {
		//	{1.0f, 0.0f, 0.0f},   // red
		//	{0.0f, 1.0f, 0.0f},   // green
		//	{0.0f, 0.0f, 1.0f},   // blue
		//	{1.0f, 1.0f, 0.0f},   // yellow
		//	{0.0f, 1.0f, 1.0f},   // cyan
		//	{1.0f, 0.0f, 1.0f},   // magenta
		//	{1.0f, 0.5f, 0.0f},   // orange
		//	{0.5f, 0.0f, 1.0f},   // purple
		//	{0.0f, 1.0f, 0.5f},   // spring green
		//	{1.0f, 0.0f, 0.5f},   // rose
		//};

		//const int colorCount = sizeof(testColors) / sizeof(testColors[0]);

		//int index = 0;
		//for (auto& actor : lights)
		//{
		//	auto act = std::dynamic_pointer_cast<CActor>(actor);
		//	auto light = act->GetComponent<CLight>();
		//	auto transform = act->GetComponent<CTransform>();

		//	if (!light || !transform)
		//		continue;

		//	// Calculate grid position on XZ plane
		//	int row = index / gridSize;
		//	int col = index % gridSize;

		//	float offsetX = (gridSize - 1) * spacing * 0.5f;
		//	float offsetZ = (gridSize - 1) * spacing * 0.5f;

		//	float x = (col * spacing) - offsetX;
		//	float z = (row * spacing) - offsetZ;

		//	transform->SetPosition(Vec3(x, 25.5f, z));

		//	
		//	
		//	float yaw = (index * 37.0f) * RADIANS_TO_DEGREES;
		//	
		//	Quaternion rotation = QMath::angleAxisRotation(yaw, Vec3(0.0f, -1.0f, 0.0f));
		//	transform->SetRotation(rotation);
		//	// Cycle through distinct colors
		//	Vec3 color = testColors[index % colorCount];
		//	light->UpdateRadius(25.0f);
		//	light->UpdateAudioId(index % 10);
		//	light->UpdateBloomScale(0.9f);
		//	light->UpdateIntensity(2.0f);
		//	light->UpdateColour(color);
		//	light->UpdateLight();

		//	index++;
		//}
		//step 1.3 Materials

		 std::vector<std::string> filepaths = { "./textures/rock.png" };

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
		auto wA = std::dynamic_pointer_cast<CActor>(world);
		auto w = wA->GetComponent<CWorld>();
		w->OnDestroy();
		w->InitializeWorld(sondId);
		auto cameraActor = std::dynamic_pointer_cast<CActor>(camera);
		Vec3 spawn = w->GetPlayerSpawn();
		float halfCylinder = cameraActor->GetComponent<CCapsuleCollider>()->GetHalfCylinder();
		spawn.y += halfCylinder * 2.0f;
		spawn.print();
		cameraActor->GetComponent<CPhysics>()->SetPosition(spawn);
		cameraActor->GetComponent<CPhysics>()->SetVelocity(Vec3(0.0f, 0.0f, 0.0f));
		auto sceneActors = engineContext.assetManager->GetActorsInScene();
		//sceneActors.push_back(camera);
		sceneActors.push_back(world);


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

std::vector<MATHEX::Plane> Scene3::GenerateFrustumPLane()
{
	std::vector<MATHEX::Plane> fusturm;
	//Matrix4 proj = camera->getProjectionMatrix() * camera->getViewMatrix();
	auto cam = std::dynamic_pointer_cast<CActor>(camera);
	auto camComp = cam->GetComponent<CCamera>();
	Matrix4 proj = camComp->GetProjectionMatrix() * camComp->GetViewMatrix();
	
	MATHEX::Plane left, right, top, bottom, near, far;
	left.x = proj[3] + proj[0];
	left.y = proj[7] + proj[4];
	left.z = proj[11] + proj[8];	
	left.d = (proj[15] + proj[12]);

	right.x = proj[3] - proj[0];
	right.y = proj[7] - proj[4];
	right.z = proj[11] - proj[8];
	right.d = (proj[15] - proj[12]);

	bottom.x = proj[3] + proj[1];
	bottom.y = proj[7] + proj[5];
	bottom.z = proj[11] + proj[9];
	bottom.d = (proj[15] + proj[13]);

	top.x = proj[3] - proj[1];
	top.y = proj[7] - proj[5];
	top.z = proj[11] - proj[9];
	top.d = (proj[15] - proj[13]);

	near.x = proj[3] + proj[2];
	near.y = proj[7] + proj[6];
	near.z = proj[11] + proj[10];
	near.d = proj[15] + proj[14];

	far.x = proj[3] - proj[2];
	far.y = proj[7] - proj[6];
	far.z = proj[11] - proj[10];
	far.d = (proj[15] - proj[14]);

	// Normalizaiont matters if we care for the actual distance
	// when we do the dot product.
	// if we are just checking below or above 0 then 
	// no need to normalize.
	left = MATHEX::PMath::normalize(left);
	right = MATHEX::PMath::normalize(right);
	bottom = MATHEX::PMath::normalize(bottom);
	top = MATHEX::PMath::normalize(top);
	near = MATHEX::PMath::normalize(near);
	far = MATHEX::PMath::normalize(far);

	fusturm.push_back(left);
	fusturm.push_back(right);
	fusturm.push_back(bottom);
	fusturm.push_back(top);
	fusturm.push_back(near);
	fusturm.push_back(far);
	return fusturm;
}

void Scene3::FrustumCheck()
{
	std::vector<MATHEX::Plane> fusturm = GenerateFrustumPLane();

	auto worldActor = std::dynamic_pointer_cast<CActor>(world);
	auto chunksData = worldActor->GetComponent<CWorld>()->GetChunkRenderData();
	for (auto& c : *chunksData) {
		//auto& c = pair.second;
		c.isCulled = false;

		for (int i = 0; i < 6; i++) {
	
			const MATHEX::Plane& p = fusturm[i];

			Vec3 pVertex;

			pVertex.x = (p.n.x > 0) ? c.aabb.max.x : c.aabb.min.x;
			pVertex.y = (p.n.y > 0) ? c.aabb.max.y : c.aabb.min.y;
			pVertex.z = (p.n.z > 0) ? c.aabb.max.z : c.aabb.min.z;

			float dot = p.n.x * pVertex.x + 
						p.n.y * pVertex.y + 
						p.n.z * pVertex.z;

			if (dot < -p.d) { // Can easly Changed to a Radius of a sphere around the Postion by -r instead of 0
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
