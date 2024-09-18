#include "EngineSubsystemScriptable.h"

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
    
}


