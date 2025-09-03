#pragma once
#include "NetAPIModule.h"

namespace Nuake 
{

	class UINetAPI : public NetAPIModule
	{
	public:
		virtual const std::string GetModuleName() const override { return "UI"; }

		virtual void RegisterMethods() override;

	};
}