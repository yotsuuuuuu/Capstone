#include "CTransform.h"
#include <MMath.h>

void CTransform::Update(const float dt)
{
}

void CTransform::SetRotation(const MATH::Quaternion& rot)
{
    rotation = rot;
    needsUBOupdate = true;
}

MATH::Matrix4 CTransform::GetTransformMatrix() const
{
    return MMath::translate(position)  * MMath::toMatrix4(rotation) * MMath::scale(scale);
}
