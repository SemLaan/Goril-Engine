#pragma once
#include "core/gorilmem.h"

namespace Goril
{

	class GraphicsContext
	{
	public:

		virtual void SwapBuffers() = 0;

		static Scope<GraphicsContext> Create(void* window);
	};
}