#pragma once
#include "IApplicationModule.h"
#include "src/Core/Core.h"
#include "LayerStack.h"

#include <string>
#include <Vector>
#include <src/Window.h>


namespace Nuake {

	struct ApplicationSpecification
	{
		std::string Name = "Application";
		uint32_t WindowWidth = 1280;
		uint32_t WindowHeight = 720;
		bool VSync = true;
		std::string WorkingDirectory = "";
		bool Headless = false;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& appSpecification);
		virtual ~Application() {};

		void Run();
		void Exit();

		virtual void OnInit()  {};
		virtual void OnUpdate() {};
		virtual void OnShutdown() {};

		virtual void OnEvent() {};

		inline const std::string& GetName() const { return m_Specification.Name; }

		void PushLayer(Scope<Layer> layer);
		void PushOverlay(Scope<Layer> layer);
	
		// Only accepts IApplicationModule type class
		template<typename T>
		T& PushModule(Scope<IApplicationModule> applicationModule)
		{
			static_assert(std::is_base_of<IApplicationModule, T>::value, "Type must be derived from IApplicationModule");

			auto* rawPtr = applicationModule.get(); // Get a raw pointer to the module.
			m_ApplicationModules.push_back(std::move(applicationModule));
			return *dynamic_cast<T*>(rawPtr);
		}

		// Only accepts IApplicationModule type class
		template <typename T>
		T& GetModule()
		{
			static_assert(std::is_base_of<IApplicationModule, T>::value, "Type must be derived from IApplicationModule");

			auto resultFindIt = std::find_if(
				m_ApplicationModules.begin(), 
				m_ApplicationModules.end(),
				[](IApplicationModule& module) 
				{
					return dynamic_cast<T*>(&module) != nullptr;
				}
			);

			if (resultFindIt == m_ApplicationModules.end())
			{
				throw std::runtime_error("Module not found.");
			}

			return dynamic_cast<T&>(*resultFindIt);
		}

		inline Ref<Window> GetWindow() const { return m_Window; }
	protected:
		ApplicationSpecification m_Specification;
		std::vector<Scope<IApplicationModule>> m_ApplicationModules;
		LayerStack m_LayerStack;
		bool m_Running;
		Ref<Window> m_Window;
	};

	// Implemented by the client
	Application* CreateApplication(int argc, char** argv);

}