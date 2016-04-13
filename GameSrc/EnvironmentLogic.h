#pragma once
#include "Global.h"

// Этот компонент прикрепляется к ноде со скайбоксом.
class EnvironmentLogic : public LogicComponent
{
    URHO3D_OBJECT(EnvironmentLogic, LogicComponent);

public:
    EnvironmentLogic(Context* context);
    static void RegisterObject(Context* context);
    void Update(float timeStep);
};
