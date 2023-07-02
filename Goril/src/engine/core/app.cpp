#include "app.h"

namespace Goril
{

	App::App()
	{
		m_layerStack = new LayerStack();
		m_timer = new Timer();
		m_previousFrameTime = m_timer->SecondsSinceStart();
	}

	App::~App()
	{
		delete m_layerStack;
		delete m_timer;
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
			float time = m_timer->SecondsSinceStart();
			float deltaTime = time - m_previousFrameTime;
			m_previousFrameTime = time;

			m_layerStack->UpdateLayers(deltaTime);
		}
	}

	void App::Quit()
	{
		m_shouldQuit = true;
	}
}