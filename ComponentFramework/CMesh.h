#pragma once
#include "Component.h"
#include "CoreStructs.h"

class CMesh : public Component
{
	const char* meshFile;
	IndexedVertexBuffer mesh;
public:
	CMesh(Ref<Component> parent_ = nullptr, const char* meshFile_ = nullptr)
		: Component(parent_), meshFile(meshFile_), mesh({}) {
	}
	virtual ~CMesh() {}
	
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}

	IndexedVertexBuffer GetMesh() const { return mesh; }

};

