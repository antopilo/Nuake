#include "EngineSubsystemScript.h"

namespace Nuake
{
 
EngineSubsystemScript::EngineSubsystemScript(const Coral::ManagedObject& object)
    : cSharpObjectInstance(object)
{

}

Coral::ManagedObject& EngineSubsystemScript::GetManagedObjectInstance()
{
    return cSharpObjectInstance;
}
}


