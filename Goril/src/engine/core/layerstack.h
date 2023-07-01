#pragma once
#include <vector>

namespace Goril
{

	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer();
		void PopLayer();

		void UpdateLayers();

	private:
		std::vector<int> m_layers;
	};
}