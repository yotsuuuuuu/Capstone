#ifndef SCENE0_H
#define SCENE0_H
#include "Scene.h"
#include "Vector.h"
#include "Renderer.h"
//#include "Camera.h"
#include "CoreStructs.h"
//temp for testing
#include "Component.h"
using namespace MATH;

/// Forward declarations 
union SDL_Event;


class Scene0 : public Scene {
private:
	

	//Camera *camera;

	
	std::vector<BufferMemory> lightsUBO;
	LightsData lights;

	bool mouseLocked = false;



	
	
	//Components
	std::vector<Ref<Component>> actorsInScene;
	Ref<Component> camera;
	Ref<Component> plane;
	Ref<Component> World;
	Ref<Component> actor;
	Ref<Component> actor1;	
	Ref<Component> shader;
	

public:

	explicit Scene0(EngineContext& context_);
	virtual ~Scene0();

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;
	virtual void HandleEvents(const SDL_Event &sdlEvent) override;
};


#endif // SCENE0_H