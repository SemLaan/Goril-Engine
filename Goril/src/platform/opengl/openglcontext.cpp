#include "openglcontext.h"

namespace Goril
{

	OpenGLContext::OpenGLContext(GLFWwindow* window)
		: m_window(window)
	{
		/* Make the window's context current */
		glfwMakeContextCurrent(m_window);

		glfwSwapInterval(1);
	}

	void OpenGLContext::SwapBuffers()
	{
		/* Swap front and back buffers */
		glfwSwapBuffers(m_window);
	}
}