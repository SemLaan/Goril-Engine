#pragma once
#include "rendering/graphicscontext.h"
#include "GLFW/glfw3.h"

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