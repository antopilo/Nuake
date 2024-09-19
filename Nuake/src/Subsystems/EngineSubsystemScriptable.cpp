#include "EngineSubsystemScriptable.h"

#include "Coral/Type.hpp"

namespace Nuake
{
 
EngineSubsystemScriptable::EngineSubsystemScriptable(const Coral::ManagedObject& object)
    : cSharpObjectInstance(object)
{

}

Coral::ManagedObject& EngineSubsystemScriptable::GetManagedObjectInstance()
{
    return cSharpObjectInstance;
}

void EngineSubsystemScriptable::Initialize()
{
    if (!cSharpObjectInstance.IsValid())
        return;

    cSharpObjectInstance.InvokeMethod("Initialize");
}

void EngineSubsystemScriptable::Tick(float deltaTime)
{
    if (!cSharpObjectInstance.IsValid())
        return;

    cSharpObjectInstance.InvokeMethod("OnTick", deltaTime);
}

void EngineSubsystemScriptable::OnScenePreInitialize(Ref<Scene> scene)
{
    if (!cSharpObjectInstance.IsValid())
        return;

    cSharpObjectInstance.InvokeMethod("InternalOnScenePreInitialize");
}

void EngineSubsystemScriptable::OnScenePostInitialize(Ref<Scene> scene)
{
    if (!cSharpObjectInstance.IsValid())
        return;

    cSharpObjectInstance.InvokeMethod("InternalOnSceneReady");
}

void EngineSubsystemScriptable::OnScenePreDestroy(Ref<Scene> scene)
{
    if (!cSharpObjectInstance.IsValid())
        return;

    if (cSharpObjectInstance.GetType().GetTypeId() == -1)
        return;
    
    cSharpObjectInstance.InvokeMethod("InternalOnScenePreDestroy");
}
}


