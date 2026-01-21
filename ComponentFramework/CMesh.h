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
	void SetMesh(const IndexedVertexBuffer& mesh_) { mesh = mesh_; }
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}

	IndexedVertexBuffer GetMesh() const { return mesh; }

};

