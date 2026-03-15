#pragma once
#include "Component.h"
#include "CTransform.h"
#include "CoreStructs.h"
class SYS_Light;

class CLight :
    public Component
{

    friend SYS_Light;
    uint32_t ssboIndex;
    CLightData data;
    Vec3 Colour;
    float radius;
    float intensity;
    Vec3 Position;
    Vec3 Direction;
    Vec2 inner_Outer;
    int Type;
    WeakRef<CTransform> transform;
    SYS_Light* LightSystem = nullptr;
   
public:
    CLight(Ref<Component> parent_, SYS_Light* sys, float radius, float intensity, Vec3 colour);
    CLight(Ref<Component> parent_, SYS_Light* sys, float radius, float intensity, Vec3 colour,
        Vec3 direction, Vec2 inner_Outer);
    ~CLight() {}

    bool OnCreate();
    void OnDestroy();
    void Update(const float dt) {}

    void UpdateColour(const Vec3& c) { Colour = c; }
    void UpdateRadius(const float& r) { radius = r; }
    void UpdateIntensity(float inten) { intensity = inten; }
    void UpdateDirection(const Vec3& dir) { Direction = dir; }
    void UpdateInner_Outer(const Vec2& in_out) { inner_Outer = in_out; }
    void UpdateType(int type) {
        if (type < 0 || type > 2) return;
        Type = type;
    }
    void UpdateLight();

private:
   
    void UpdateData();
    CLightData GetUpdatedData();
};

