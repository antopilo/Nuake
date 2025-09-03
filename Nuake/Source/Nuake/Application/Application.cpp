#include "Application.h"
#include "Layer.h"

#include <filesystem>

#include <Tracy.hpp>

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
		while (!m_Window->ShouldClose())
		{
			ZoneScoped;

			for (auto& layer : m_LayerStack)
			{
				ZoneScopedN("Layer Update");
				layer->OnUpdate();
			}

			if (!m_Specification.Headless)
			{
				for (auto& layer : m_LayerStack)
				{
					ZoneScopedN("Layer Draw");
					layer->OnDraw();
				}
			}

			FrameMark;
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
