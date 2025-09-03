#pragma once

namespace NuakeUI
{
	class IController
	{
	public:
		virtual void OnRegister() = 0;
		virtual void OnUnregister() = 0;
	};
}