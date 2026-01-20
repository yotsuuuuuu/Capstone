#pragma once
#include "Component.h"
#include "CoreStructs.h"

class Renderer;

class CMesh : public Component
{
	const char* meshFile;
	Renderer* renderer;
	IndexedVertexBuffer mesh;
public:
	CMesh(Ref<Component> parent_,Renderer* renderer_, const char* meshFile_ = nullptr)
		: Component(parent_),renderer(renderer_), meshFile(meshFile_), mesh({}) {
	}
	virtual ~CMesh() {}
	
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}

	IndexedVertexBuffer GetMesh() const { return mesh; }

};

