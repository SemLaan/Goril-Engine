#include "app.h"
#include "rendering/lowlevelrenderer/rendercommand.h"

namespace Goril
{

	App::App(AppProperties& appProperties)
	{
		Window::Get()->Init(appProperties.m_width, appProperties.m_height);
		LLR::RenderCommand::Init();
		m_layerStack = new LayerStack();
		m_timer = new Timer();
		m_previousFrameTime = m_timer->SecondsSinceStart();
	}

	App::~App()
	{
		delete m_layerStack;
		delete m_timer;
		LLR::RenderCommand::Shutdown();
		Window::Get()->Shutdown();
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