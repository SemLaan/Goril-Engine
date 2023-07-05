#include "glewwindow.h"

namespace Goril
{

	void GlewWindow::Init(unsigned int width, unsigned int height)
	{
		m_windowSize.width = width;
		m_windowSize.height = height;

		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		/* Create a windowed mode window and its OpenGL context */
		m_window = glfwCreateWindow(width, height, "window title", NULL, NULL);

		/* Make the window's context current */
		glfwMakeContextCurrent(m_window);

		glfwSwapInterval(1);
	}

	void GlewWindow::Shutdown()
	{
		glfwTerminate();
	}

	void GlewWindow::Present()
	{
		/* Swap front and back buffers */
		glfwSwapBuffers(m_window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	WindowSize GlewWindow::GetWindowSize() const
	{
		return m_windowSize;
	}
}
