#pragma once
#include "core/window.h"
#include "GLFW/glfw3.h"
#include "rendering/graphicsdeviceinterface/graphicscontext.h"

namespace Goril
{

	class GlewWindow : public Window
	{
	public:
		// Inherited via Window
		virtual void Init(unsigned int width, unsigned int height) override;
		virtual void Shutdown() override;
		virtual void Present() override;
		virtual WindowSize GetWindowSize() const override;
	private:
		GLFWwindow* m_window;
		Scope<GraphicsContext> m_graphicsContext;
		WindowSize m_windowSize;
	};
}
