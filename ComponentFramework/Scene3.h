#ifndef Scene3_H
#define Scene3_H
#include "Scene.h"
#include "Vector.h"
#include "Renderer.h"
//#include "Camera.h"
#include "CoreStructs.h"
//temp for testing
#include "Component.h"
#include "TerrainPreset.h"
using namespace MATH;

/// Forward declarations 
union SDL_Event;


class Scene3 : public Scene {
private:
	

	//Camera *camera;

	
	std::vector<BufferMemory> lightsUBO;
	LightsData lights;

	bool mouseLocked = false;


	TerrainPreset preset;
	TerrainPreset preset2;
	TerrainPreset preset3;
	
	
	
	//Components
	std::vector<Ref<Component>> actorsInScene;
	Ref<Component> camera;
	Ref<Component> shader;
	Ref<Component> world;
	
	

public:

	explicit Scene3(EngineContext& context_);
	virtual ~Scene3();

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float deltaTime) override;
	virtual void Render() const override;
	virtual void HandleEvents(const SDL_Event &sdlEvent) override;
};


#endif // Scene3_H