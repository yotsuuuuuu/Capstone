#include "CCollider.h"
#include "CTransform.h"
#include "CActor.h"



bool CCollider::OnCreate()
{
    if (isCreated) return true;

    auto actor = parent.lock();
    if (!actor) return false;

    // Ensure we have a transform component
    auto t = std::dynamic_pointer_cast<CActor>(actor)->GetComponent<CTransform>();
    if (!t) {
        // Can't have collider without transform
        return false;
    }

	transform = t;

    isCreated = true;
    return true;
}

void CCollider::OnDestroy()
{
    isCreated = false;
}

void CCollider::Update(const float dt)
{
}
