#include "CLight.h"
#include "CActor.h"
#include "SYS_Light.h"

CLight::CLight(Ref<Component> parent_, SYS_Light* sys, float radius_, float intensity_, Vec3 colour_)
	:Component(parent_), LightSystem(sys),radius(radius_),intensity(intensity_),Colour(colour_),Type(0){
	
}

CLight::CLight(Ref<Component> parent_, SYS_Light* sys, float radius_, float intensity_, Vec3 colour_, 
	Vec3 direction_, Vec2 inner_Outer_)
	:Component(parent_), LightSystem(sys), radius(radius_), intensity(intensity_), Colour(colour_), Type(1),
	Direction(direction_),inner_Outer(inner_Outer_){
		
}

bool CLight::OnCreate() {
	if (isCreated)
		return true;

	auto a = parent.lock();
	if (!LightSystem && !a)
		return false;

	auto T = std::dynamic_pointer_cast<CActor>(a)->GetComponent<CTransform>();
	if (!T)
		return false;
	transform = T;

	if (!LightSystem->RegisterLight(this)) // assume that it populates the data when resgisters
		return false;


	isCreated = true;
	return true;

}

void CLight::OnDestroy() {

	if (!isCreated)
		return;
	if (!LightSystem)
		return;

	LightSystem->DeregisterLight(this);
	isCreated = false;
}



void CLight::UpdateLight()
{
	if (!isCreated)
		return;
	if (!LightSystem)
		return;

	LightSystem->UpdateLightData(this);
}

void CLight::UpdateData()
{
	Vec3 Position;
	if (auto T = transform.lock()) {
		Position = T->GetPosition();
	}
	data.colour_intensity = Vec4(Colour, intensity);
	data.position_radius = Vec4(Position, radius);
	data.direction_inner = Vec4(Direction, inner_Outer.x);
	data.outer_type_pad = Vec4(inner_Outer.y, static_cast<float>(Type), 0, 0);
}

CLightData CLight::GetUpdatedData()
{
	UpdateData();
	return data;
}
