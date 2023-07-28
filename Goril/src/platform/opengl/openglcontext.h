#pragma once
#include "rendering/graphicscontext.h"

struct GLFWwindow;

namespace Goril
{

	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* window);

		virtual void SwapBuffers() override;
	private:
		GLFWwindow* m_window;
	};
}