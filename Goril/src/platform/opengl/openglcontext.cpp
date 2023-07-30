#include "openglcontext.h"
#include <core/core.h>

namespace Goril
{

	OpenGLContext::OpenGLContext(GLFWwindow* window)
		: m_window(window)
	{
		/* Make the window's context current */
		glfwMakeContextCurrent(m_window);

		int succes = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		GRASSERT(succes);

		GRINFO("OpenGL Info:");
		GRINFO("  Vendor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
		GRINFO("  Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
		GRINFO("  Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

		glfwSwapInterval(1);
	}

	void OpenGLContext::SwapBuffers()
	{
		/* Swap front and back buffers */
		glfwSwapBuffers(m_window);
	}
}