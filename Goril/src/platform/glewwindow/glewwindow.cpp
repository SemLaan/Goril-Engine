#include "glewwindow.h"
#include "core/core.h"

namespace Goril
{

	void GlewWindow::Init(unsigned int width, unsigned int height)
	{
		m_windowSize.width = width;
		m_windowSize.height = height;

		int succes = glfwInit();
		GRASSERT(succes);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		/* Create a windowed mode window and its OpenGL context */
		m_window = glfwCreateWindow(width, height, "window title", NULL, NULL);
		GRASSERT(m_window);

		m_graphicsContext = GraphicsContext::Create(m_window);

		GRINFO("Window and context initialized");
	}

	void GlewWindow::Shutdown()
	{
		m_graphicsContext.reset();
		glfwDestroyWindow(m_window);
		glfwTerminate();
		GRINFO("Window and context shutdown");
	}

	void GlewWindow::Present()
	{
		m_graphicsContext->SwapBuffers();

		/* Poll for and process events */
		glfwPollEvents();
	}

	WindowSize GlewWindow::GetWindowSize() const
	{
		return m_windowSize;
	}
}
