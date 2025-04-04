#pragma once
#include "NetAPIModule.h"

namespace Nuake {

	class ModulesAPI : public NetAPIModule
	{
	public:
		virtual const std::string GetModuleName() const override { return "Modules"; }

		virtual void RegisterMethods() override;

	};
}