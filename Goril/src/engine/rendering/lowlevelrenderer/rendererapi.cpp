#include "rendererapi.h"
#include "opengl/openglrendererapi.h"

namespace Goril::LLR
{
	API RendererAPI::s_API = API::OpenGL;

	Scope<RendererAPI> RendererAPI::Create()
	{
		switch (GetAPIType())
		{
		case API::OpenGL:
			return CreateScope<OpenGL::OpenGLRendererAPI>();
		default:
			return nullptr;
		}
	}
}