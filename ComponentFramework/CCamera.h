#pragma once
#include "Component.h"
#include "CoreStructs.h"


class CTransform;
class Renderer;

class CCamera : public Component {

	WeakRef<CTransform> transform;
	Matrix4 projectionMatrix;
	Matrix4 viewMatrix;
	float fovy;
	float aspectRatio;
	float nearClip;
	float farClip;
	
	std::vector<BufferMemory> cameraUBO;
	Renderer* renderer;

	//bool needsUpdate = false;
	
public:
	CCamera(Ref<Component> parent_, Renderer* renderer_, const float& FOVY, const float& aspectRatio_, const float& nearClip_, const float& farClip_)
		: Component(parent_), renderer(renderer_), fovy(FOVY), aspectRatio(aspectRatio_), nearClip(nearClip_), farClip(farClip_) {
	
	}
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}
	
	std::vector<BufferMemory> GetCameraUBO() { return cameraUBO; }
	Matrix4 GetProjection() { return projectionMatrix; }
	Matrix4 GetViewMatrix() { return viewMatrix; }
	Vec2 GetzPlanes() { return Vec2(nearClip, farClip); }

	void UpdateUBO(uint32_t uboindex);

	void UpdateProjectionMatrix(float FOVY,float aspectRatio,float nearClip,float farClip);
	std::vector<float> GetProjMatrixValues();
	void UpdateViewMatrix();

	Matrix4 GetProjectionMatrix() const { return projectionMatrix; }
	Matrix4 GetViewMatrix() const { return viewMatrix; }


	// input stuff
	MATH::Vec3 GetFrontVector() const;
	MATH::Vec3 GetRightVector() const;
	MATH::Vec3 GetUpVector() const;

};