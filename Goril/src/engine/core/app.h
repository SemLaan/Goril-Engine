#pragma once
#include "layerstack.h"
#include "gorilmem.h"
#include "timer.h"
#include "window.h"

namespace Goril
{

	struct AppProperties
	{
		unsigned int m_width;
		unsigned int m_height;
	};

	class App 
	{
	public:
		App(AppProperties& appProperties);
		virtual ~App();

		App& operator=(const App&) = delete;
		App(const App&) = delete;
		App& operator=(App&&) = delete;
		App(App&&) = delete;


		void PushLayer(Ref<Layer> layer);
		void PopLayer();

		void Run();
		void Quit();

	private:
		bool m_shouldQuit = false;
		LayerStack* m_layerStack;
		Timer* m_timer;
		float m_previousFrameTime;
	};
}