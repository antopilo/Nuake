#pragma once
#include "NetAPIModule.h"

namespace Nuake {

	class EngineNetAPI : public NetAPIModule
	{
	public:
		virtual const std::string GetModuleName() const override { return "Engine"; }

		virtual void RegisterMethods() override;

	};
}