#pragma once
#include "layerstack.h"
#include "gorilmem.h"

namespace Goril
{
	class App 
	{
	public:
		App();
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
	};
}