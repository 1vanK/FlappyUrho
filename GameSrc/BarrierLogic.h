#pragma once
#include "Global.h"

// Этот компонент прикрепляется к нодам с барьерами.
// Барьер содержит две дочерние ноды - верхнюю и нижнюю трубы.
class BarrierLogic : public LogicComponent
{
    URHO3D_OBJECT(BarrierLogic, LogicComponent);

public:
    BarrierLogic(Context* context);
    static void RegisterObject(Context* context);
    void Update(float timeStep);
};
