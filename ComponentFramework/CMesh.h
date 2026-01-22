#pragma once
#include "Component.h"
#include "CoreStructs.h"



class CMesh : public Component
{
	const char* meshFile;

	IndexedVertexBuffer mesh;
public:
	CMesh(Ref<Component> parent_, const char* meshFile_ = nullptr)
		: Component(parent_), meshFile(meshFile_), mesh({}) {
	}
	virtual ~CMesh() {}
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}

	void SetMesh(const IndexedVertexBuffer& mesh_) { mesh = mesh_; }
	IndexedVertexBuffer GetMesh() const { return mesh; }
	
};

