#pragma once
#include "Chunk.h"
#include "Component.h" 


class CChunk : public Component 
{
	Chunk chunk;

public:

	CChunk(Ref<Component> parent_ = nullptr, Vec2 pos) : Component(parent_) { chunk = Chunk(pos); }
	CChunk(Vec2 pos) { chunk = Chunk(pos); }
	virtual ~CChunk() {}

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void Update(const float dt) = 0;

	const std::vector<float>& GetHeightmap() const { return chunk.GetHeightmap(); }
	const Vec2& GetPosition() const { return chunk.GetPosition(); }

	void SetHeightmap(std::vector<float>&& hm) { chunk.SetHeightmap(std::move(hm)); }
	Vec2 getChunkPos() const { return chunk.getChunkPos(); }
};

