#include "CTransform.h"
#include <MMath.h>

MATH::Matrix4 CTransform::GetTransformMatrix() const
{
    return MMath::translate(position)  * MMath::toMatrix4(rotation) * MMath::scale(scale);
}
