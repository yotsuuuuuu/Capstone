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
	
	std::vector<BufferMemory> cameraUBO;
	bool uboNeedsUpdate = false;

public:
	CCameraActor(Ref<Component> parent_) : CActor(parent_){}

	void UpdateProjectionMatrix(const float& FOVY, const float& aspectRatio, const float& nearClip, const float& farClip);
	void UpdateViewMatrix();
	void SetCameraUBO(const std::vector<BufferMemory>& ubo_) { cameraUBO = ubo_; }
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}
	MATH::Matrix4 GetViewMatrix() const { return viewMatrix; }
	MATH::Matrix4 GetProjectionMatrix() const { return projectionMatrix; }
	CameraData GetCamDataUBO();
	bool IsUBOOutDated() const { return uboNeedsUpdate; }
};

