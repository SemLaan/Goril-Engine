#include "app.h"
#include "rendering/lowlevelrenderer/renderapi.h"
#include "logger.h"

namespace Goril
{

	App::App(AppProperties& appProperties)
	{
		Logger::Get()->Init();
		Window::Get()->Init(appProperties.m_width, appProperties.m_height);
		LLR::RenderAPI::Get()->Init();
		m_layerStack = new LayerStack();
		m_timer = new Timer();
		m_previousFrameTime = m_timer->SecondsSinceStart();
	}

	App::~App()
	{
		delete m_layerStack;
		delete m_timer;
		LLR::RenderAPI::Get()->Shutdown();
		Window::Get()->Shutdown();
		Logger::Get()->Shutdown();
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

			Window::Get()->Present();
		}
	}

	void App::Quit()
	{
		m_shouldQuit = true;
	}
}