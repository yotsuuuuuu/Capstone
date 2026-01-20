#pragma once
#include "CActor.h"
#include "CoreStructs.h"

class Renderer;

class CCameraActor :
    public CActor
{
	// TODO: creationg and up dating of UBOs for camera data
	// thought maybe there should 
	// be a ubo component
	MATH::Matrix4 viewMatrix;
	MATH::Matrix4 projectionMatrix;
	Renderer* renderer;
	std::vector<BufferMemory> cameraUBO;

public:
	CCameraActor(Ref<Component> parent_,Renderer* renderer_) : CActor(parent_),renderer(renderer_) {}

	void UpdateProjectionMatrix(const float& FOVY, const float& aspectRatio, const float& nearClip, const float& farClip);
	void UpdateViewMatrix();
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}
	MATH::Matrix4 GetViewMatrix() const { return viewMatrix; }
	MATH::Matrix4 GetProjectionMatrix() const { return projectionMatrix; }
	void UpdateUBO();
private:
	CameraData GetCamDataUBO();
};

