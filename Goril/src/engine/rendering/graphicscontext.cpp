#include "graphicscontext.h"
#include "opengl/openglcontext.h"

namespace Goril
{
	GraphicsContext* GraphicsContext::Create(void* window)
	{
		return new OpenGLContext(static_cast<GLFWwindow*>(window));
	}
}