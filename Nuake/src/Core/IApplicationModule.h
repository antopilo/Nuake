#pragma once
#include <string>

namespace Nuake
{
	class IApplicationModule
	{
	private:
		std::string m_Name;

	public:
		IApplicationModule(const std::string& name) : m_Name(name) {}
		virtual ~IApplicationModule() {}

		virtual void OnInit() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnExit() = 0;
	};

}