#pragma once

#include "Entity.h"
#include "../Scene/Entities/Components/ParentComponent.h"
#include "../Scene/Entities/Components/NameComponent.h"

class ScriptableEntity
{
public:

	virtual void OnCreate() {}
	virtual void OnDestroy() {}
	virtual void OnUpdate(Timestep ts){}

};