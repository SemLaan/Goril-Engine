#include "layerstack.h"


namespace Goril
{

	LayerStack::LayerStack()
	{
	}

	LayerStack::~LayerStack()
	{
	}

	void LayerStack::PushLayer(Ref<Layer> layer)
	{
		m_layers.push_back(layer);
		layer->OnAttach();
	}

	void LayerStack::PopLayer()
	{
		m_layers[m_layers.size() - 1]->OnDetach();
		m_layers.pop_back();
	}

	void LayerStack::UpdateLayers()
	{
		for (Ref<Layer>& layer : m_layers)
		{
			layer->UpdateLayer();
		}
	}
}