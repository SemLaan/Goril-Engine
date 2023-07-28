#include "graphicscontext.h"
#include "opengl/openglcontext.h"

namespace Goril
{
	Scope<GraphicsContext> GraphicsContext::Create(void* window)
	{
		return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
	}
}