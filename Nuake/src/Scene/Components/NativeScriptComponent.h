#pragma once
#include <functional>
#include "../Core/Timestep.h"
#include "../Entities/ScriptableEntity.h"
struct NativeScriptComponent
{
	ScriptableEntity* Instance = nullptr;

	ScriptableEntity *(*InstantiateScript)();
	void (*DestroyScript)(NativeScriptComponent*);

	template<typename T>
	void Bind()
	{
		InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
		DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
	}
};