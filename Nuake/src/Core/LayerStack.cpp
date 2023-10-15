#include "LayerStack.h"
#include "Layer.h"

namespace Nuake {

	LayerStack::LayerStack()
	{
		m_LayerInsert = m_Layers.begin();
	}

	void LayerStack::PushLayer(Ref<Layer> layer)
	{
		m_LayerInsert = m_Layers.emplace(m_LayerInsert, std::move(layer));
	}

	void LayerStack::PushOverlayLayer(Ref<Layer> layer)
	{
		m_Layers.emplace_back(std::move(layer));
	}

	void LayerStack::PopLayer(Ref<Layer> layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
			m_LayerInsert--;
		}
	}

	void LayerStack::PopOverlayLayer(Ref<Layer> layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
		}
	}

}