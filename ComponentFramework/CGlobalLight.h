#pragma once
#include "Component.h"
#include "CoreStructs.h"


class CTransform;
class Renderer;
// TODO: ADJUST FOR MULTIPLE LIGHTS view and projection mattries and ubos
class CGlobalLight : public Component
{
	enum GLMODE {ORTHO,PRESPECTIVE};
	OrthConfig Othc;
	float MedFactor = 2.0f;
	float LowFactor = 4.0f;
	PerspectiveConfig Perc;
	GLMODE mode;
	WeakRef<CTransform> transform;
	GlobalLightData G_data;
	// uniforms for each view * porj matrix for each  high med and low shadow map per frame so 6 ubos
	// only one half is update each frame
	std::vector<BufferMemory> ShadowsUBOs; 
	// only need 2 ubo pre frame but contains copy current frame all matrix for the frame
	// aka take the same data used the shadow this frame and copy them here.
	std::vector<BufferMemory> GL_UBO;
	MATH::Quaternion orientation;
	float distance;
	Renderer* renderer;
public:
	CGlobalLight(Ref<Component> parent_, Renderer* renderer_, OrthConfig config, LightConfig data);
	CGlobalLight(Ref<Component> parent_, Renderer* renderer_, PerspectiveConfig config, LightConfig data);

	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}

	std::vector<BufferMemory> GetMainUBO() { return GL_UBO; }
	std::vector<BufferMemory> GetShadowUBO() { return ShadowsUBOs; }

	void SetLightData(LightConfig data);
	void SetLightProjection(OrthConfig config);
	void SetLightProjection(PerspectiveConfig config);

	void UpdateUBO(uint32_t uboindex);

private:
	void UpdateViewMatrix();
};

