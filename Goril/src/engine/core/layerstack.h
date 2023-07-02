#pragma once
#include <vector>
#include "layer.h"
#include "gorilmem.h"

namespace Goril
{

	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Ref<Layer> layer);
		void PopLayer();

		void UpdateLayers(float deltaTime);

		LayerStack& operator=(const LayerStack&) = delete;
		LayerStack(const LayerStack&) = delete;
		LayerStack& operator=(LayerStack&&) = delete;
		LayerStack(LayerStack&&) = delete;

	private:
		std::vector<Ref<Layer>> m_layers;
	};
}