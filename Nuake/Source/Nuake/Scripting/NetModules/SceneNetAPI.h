#pragma once
#include "NetAPIModule.h"

namespace Nuake {

	class SceneNetAPI : public NetAPIModule
	{
	public:
		virtual const std::string GetModuleName() const override { return "Scene"; }

		virtual void RegisterMethods() override;

	};
}