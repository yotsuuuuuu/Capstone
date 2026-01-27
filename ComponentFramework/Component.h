#pragma once
#include <memory>

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T>
using WeakRef = std::weak_ptr<T>;

class Component{
protected:
	Ref<Component> parent;
	bool isCreated;
public:
	Component(Ref<Component> parent_ = nullptr) :parent(parent_), isCreated(false) {}
	virtual ~Component() {}

	virtual bool OnCreate() = 0;
	virtual void OnDestroy() = 0;
	virtual void Update(const float dt) = 0;
};

