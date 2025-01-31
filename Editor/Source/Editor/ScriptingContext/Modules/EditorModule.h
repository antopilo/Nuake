#pragma once
#include "Nuake/Scripting/NetModules/NetAPIModule.h"

class EditorNetAPI : public Nuake::NetAPIModule
{
public:
	virtual const std::string GetModuleName() const override { return "Editor"; }

	virtual void RegisterMethods() override;

};