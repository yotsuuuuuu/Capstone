#ifndef SCENE2_H
#define SCENE2_H
#include "Scene.h"
#include "Vector.h"
#include "Renderer.h"
//#include "Camera.h"
#include "CoreStructs.h"
//temp for testing
#include "Component.h"
using namespace MATH;
#include "World.h"

/// Forward declarations 
union SDL_Event;


class Scene2 : public Scene {
private:

	
	//Camera *camera;

	
	std::vector<BufferMemory> lightsUBO;
	LightsData lights;

	World* world;
	bool mouseLocked = true;


	
	
	//Components
	Ref<Component> player;
	//Ref<CPlayerActor> cPlayer;
	Ref<Component> actor;
	Ref<Component> actor1;	
	Ref<Component> shader;

	//Ref<CInput> playerController; 

public:

	explicit Scene2(EngineContext& context_);
	virtual ~Scene2();

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;
	virtual void HandleEvents(const SDL_Event &sdlEvent) override;
};


#endif // SCENE2_H