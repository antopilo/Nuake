#pragma once
#include "Nuake/Core/Core.h"


#include <vector>


namespace Nuake {

	class Layer;


	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack() = default;

		void PushLayer(Ref<Layer> layer);
		void PopLayer(Ref<Layer> layer);

		void PushOverlayLayer(Ref<Layer> layer);
		void PopOverlayLayer(Ref<Layer> layer);

		std::vector<Ref<Layer>>::iterator begin() { return m_Layers.begin(); }
		std::vector<Ref<Layer>>::iterator end() { return m_Layers.end(); }

	private:
		std::vector<Ref<Layer>> m_Layers;
		std::vector<Ref<Layer>>::iterator m_LayerInsert;
	};

}