#pragma once
#include "Component.h"
#include "CoreStructs.h"
#include <string>


class Renderer;
class CMesh : public Component
{
	std::string meshFile;

	IndexedVertexBuffer mesh;
	Renderer* render;
public:
	CMesh(Ref<Component> parent_, Renderer* render_, std::string meshFile_ = nullptr)
		: Component(parent_),render(render_), meshFile(meshFile_), mesh({}) {
	}
	virtual ~CMesh() {}
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override {}

	void SetMesh(const IndexedVertexBuffer& mesh_) { mesh = mesh_; }
	IndexedVertexBuffer GetMesh() const { return mesh; }
	
};

