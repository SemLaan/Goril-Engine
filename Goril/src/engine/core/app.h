#pragma once
#include "layerstack.h"
#include "gorilmem.h"

namespace Goril
{
	class App 
	{
	public:
		App();
		~App();

		void Run();

	private:
		Scope<LayerStack> m_layerStack;
	};
}