#pragma once

#include "Entity.h"
#include "../Components/ParentComponent.h"
#include "../Components/NameComponent.h"

class ScriptableEntity
{
public:

	virtual void OnCreate() {}
	virtual void OnDestroy() {}
	virtual void OnUpdate(Timestep ts){}

};