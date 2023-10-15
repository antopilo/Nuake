#pragma once
#include "Core.h"
#include "Application.h"


namespace Nuake {

	class Layer
	{
	public:
		Layer(const std::string& name = "Layer") : m_Name(name) { };
		virtual ~Layer() = default;

		virtual void OnAttach() {};
		virtual void OnUpdate() {};
		virtual void OnDetach() {};

		virtual void OnDraw() {};

		// TODO: OnEvent

		inline const std::string& GetName() const { return m_Name; }
	
		// This is used for layer to have access to the
		// application that owns them and their modules.
		void SetApplicationOwner(Application* application) { m_Application = application; }
		Application& GetApplication() { return *m_Application; }

	private:
		std::string m_Name;

	protected:
		Application* m_Application;
	};
}