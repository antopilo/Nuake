#include "EngineSubsystemScriptable.h"

#include "Nuake/Scripting/ScriptingEngineNet.h"

#include <Coral/Type.hpp>

namespace Nuake
{
 
EngineSubsystemScriptable::EngineSubsystemScriptable(const Coral::ManagedObject& object)
    : cSharpObjectInstance(object)
{

}

EngineSubsystemScriptable::~EngineSubsystemScriptable()
{
    if (!cSharpObjectInstance.IsValid())
        return;

    ScriptingEngineNet::Get().OnUninitialize().Remove(scriptEngineUninitializeDelegateHandle);
    
    cSharpObjectInstance.Destroy();
}

Coral::ManagedObject& EngineSubsystemScriptable::GetManagedObjectInstance()
{
    return cSharpObjectInstance;
}

void EngineSubsystemScriptable::Initialize()
{
    scriptEngineUninitializeDelegateHandle = ScriptingEngineNet::Get().OnUninitialize().AddRaw(this, &EngineSubsystemScriptable::OnScriptEngineUninitialize);

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
    
    cSharpObjectInstance.InvokeMethod("InternalOnScenePreDestroy");
}

void EngineSubsystemScriptable::OnScriptEngineUninitialize()
{
    if (!cSharpObjectInstance.IsValid())
        return;
    
    cSharpObjectInstance.Destroy();
}
    
}


