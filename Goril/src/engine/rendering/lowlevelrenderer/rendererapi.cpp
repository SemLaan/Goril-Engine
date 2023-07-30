#include "rendererapi.h"
#include "opengl/openglrendererapi.h"

namespace Goril::LLR
{
	API RendererAPI::s_API = API::OpenGL;

	RendererAPI*& RendererAPI::Get()
	{
		static RendererAPI* instance = nullptr;
		if (!instance)
		{
			switch (GetAPIType())
			{
			case API::OpenGL:
				instance = new OpenGL::OpenGLRendererAPI();
				break;
			default:
				instance = nullptr;
				break;
			}
		}
		return instance;
	}
}