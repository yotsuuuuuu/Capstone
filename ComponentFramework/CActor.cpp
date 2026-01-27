#include "CActor.h"
#include "CTransform.h"
bool CActor::OnCreate()
{
    if (isCreated)
        return true;
   
    bool flag = false;
    for(const auto & comp : components)
    {
        if(comp->OnCreate() == false)
        {
            flag = true;
            break;
        }
	}
    isCreated = flag;

    return isCreated;
}

void CActor::OnDestroy()
{
    for (const auto& comp : components)
    {
        comp->OnDestroy();
    }
	isCreated = false;
}

void CActor::Update(const float dt)
{
}

void CActor::DeleteComponents()
{
    components.clear();
}

void CActor::ListComponents() const
{
	Debug::Info("Listing Components for Actor:", __FILE__, __LINE__);
    for (const auto& comp : components)
    {
        Debug::Info("Component: " + std::string(typeid(*comp).name()), __FILE__, __LINE__);
	}
}

MATH::Matrix4 CActor::GetModelMatrix() const
{
	MATH::Matrix4 modelMatrix;
	Ref<CTransform> transform = GetComponent<CTransform>();
    if (transform) {
		modelMatrix = transform->GetTransformMatrix();
    }

    if (parent) {
        Ref<CActor> parentActor =  std::dynamic_pointer_cast<CActor>(parent);
        if (parentActor) {
            modelMatrix = parentActor->GetModelMatrix() * modelMatrix;
		}
    }
    return modelMatrix;
}
