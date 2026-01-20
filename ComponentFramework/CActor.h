#pragma once
#include "Component.h"
#include <vector>
#include <Matrix.h>
#include "Debug.h"

class CActor : public Component {
	// to avoid the auto builded in copy constructor
	// not sure if all components should have this behavior
	// but only actor has it for now
	CActor(const CActor&) = delete;
	CActor(CActor&&) = delete;
	CActor& operator=(const CActor&) = delete;
	CActor& operator=(CActor&&) = delete;
protected:
	std::vector<Ref<Component>> components;
public:
	CActor(Ref<Component> parent_ = nullptr) : Component(parent_) {}
	virtual ~CActor() {}
	
	virtual bool OnCreate() override;
	virtual void OnDestroy() override;
	virtual void Update(const float dt) override;

	void DeleteComponents();
	void ListComponents() const;

	MATH::Matrix4 GetModelMatrix() const;

	Ref<Component> GetParent() const { return parent; }

	template<typename T, typename... TArgs>
	inline void AddComponent(TArgs&&... args);

	template<typename T>
	inline void AddComponent(Ref<T> comp);

	template<typename T>
	inline Ref<T> GetComponent() const;

	template<typename T>
	inline void RemoveComponent();


};

template<typename T, typename... TArgs>
inline void CActor::AddComponent(TArgs&&... args) 
{
	if (GetComponent<T>()) {
#ifdef _DEBUG
		Debug::Error("Trying to add duplicate component to actor", __FILE__, __LINE__);
#endif 
		return;
	}
	Ref<T> newComp = std::make_shared<T>(std::forward<TArgs>(args)...);
	components.push_back(newComp);
}

template<typename T>
inline void CActor::AddComponent(Ref<T> comp) {
	if (comp) {
#ifdef _DEBUG
		Debug::Error("Not valid component", __FILE__, __LINE__);
#endif 
		return;
	}
	if (GetComponent<T>()) {
#ifdef _DEBUG
		Debug::Error("Trying to add duplicate component to actor", __FILE__, __LINE__);
#endif 
		return;
	}
	components.push_back(comp);
}

template<typename T>
inline Ref<T> CActor::GetComponent() const {
	for ( const auto& comp : components) {
		Ref<T> castedComp = std::dynamic_pointer_cast<T>(comp);
		if (castedComp) {
			return castedComp;
		}
	}
	return nullptr;
}

template<typename T>
inline void CActor::RemoveComponent() {
	for (auto it = components.begin(); it != components.end(); ++it) {
		 if( std::dynamic_pointer_cast<T>(*it) ) {
			 components.erase(it);
			 return;
		 }
	}
}