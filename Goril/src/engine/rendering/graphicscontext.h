#pragma once

namespace Goril
{

	class GraphicsContext
	{
	public:

		virtual void SwapBuffers() = 0;

		static GraphicsContext* Create(void* window);
	};
}