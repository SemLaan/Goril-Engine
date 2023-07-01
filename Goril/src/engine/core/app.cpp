#include "app.h"

namespace Goril
{

	App::App()
	{
		m_layerStack = CreateScope<LayerStack>();
	}

	App::~App()
	{
	}

	void App::Run()
	{
		while (true)
		{
			m_layerStack->UpdateLayers();
		}
	}
}