#pragma once
#include "src/Core/Core.h"
#include "src/Core/Logger.h"

#include <Coral/NativeString.hpp>

namespace Nuake {

	class NetAPIModule
	{
	public:
		virtual const std::string GetModuleName() const = 0;
		virtual void RegisterMethods() = 0;

		std::unordered_map<std::string, void*> GetMethods() const { return m_Methods; }
	protected:
		void RegisterMethod(const std::string& name, void* methodPtr)
		{
			m_Methods.emplace(name, methodPtr);
		}

	private:
		std::unordered_map<std::string, void*> m_Methods;
	};
}