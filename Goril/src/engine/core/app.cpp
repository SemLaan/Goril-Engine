#include "app.h"

namespace Goril
{

	App::App()
	{
		m_layerStack = new LayerStack();
	}

	App::~App()
	{
		delete m_layerStack;
	}

	void App::PushLayer(Ref<Layer> layer)
	{
		m_layerStack->PushLayer(layer);
	}

	void App::PopLayer()
	{
		m_layerStack->PopLayer();
	}

	void App::Run()
	{
		while (!m_shouldQuit)
		{
			m_layerStack->UpdateLayers();
		}
	}

	void App::Quit()
	{
		m_shouldQuit = true;
	}
}