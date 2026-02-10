#pragma once
#include "Component.h"
#include "CoreStructs.h"


class CTransform;
class Renderer;

class CGlobalLight : public Component
{
	WeakRef<CTransform> transform;
	GlobalLightData G_data;
	std::vector<BufferMemory> GL_UBO;
	Renderer* renderer;
public:
	CGlobalLight(Ref<Component> parent_, Renderer* renderer_, OrthConfig config, LightConfig data);
	CGlobalLight(Ref<Component> parent_, Renderer* renderer_, PerspectiveConfig config, LightConfig data);

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}

	std::vector<BufferMemory> GetUBO() { return GL_UBO; }

	void SetLightData(LightConfig data);
	void SetLightProjection(OrthConfig config);
	void SetLightProjection(PerspectiveConfig config);

	void UpdateUBO(uint32_t uboindex);

private:
	void UpdateViewMatrix();
};

