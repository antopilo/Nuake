#include "Application.h"
#include "Layer.h"

#include <filesystem>

namespace Nuake {

	Application::Application(const ApplicationSpecification& appSpecification)
		: m_Specification(appSpecification),
		m_Running(true)
	{
		if (!appSpecification.Headless)
		{
			m_Window = Window::Get();
		}
	}

	void Application::PushLayer(Scope<Layer> layer)
	{
		layer->OnAttach();
		layer->SetApplicationOwner(this);
		m_LayerStack.PushLayer(std::move(layer));
	}

	void Application::PushOverlay(Scope<Layer> layer)
	{
		layer->OnAttach();
		layer->SetApplicationOwner(this);
		m_LayerStack.PushOverlayLayer(std::move(layer));
	}

	void Application::Run()
	{
		while (m_Running)
		{
			for (auto& layer : m_LayerStack)
			{
				layer->OnUpdate();
			}

			if (!m_Specification.Headless)
			{
				for (auto& layer : m_LayerStack)
				{
					layer->OnDraw();
				}
			}
		}
	}

	void Application::Exit()
	{
		for (auto& layer : m_LayerStack)
		{
			layer->OnDetach();
		}
	}
}