#include "renderapi.h"
#include "opengl/openglrenderapi.h"

namespace Goril::LLR
{
	API RenderAPI::s_API = API::OpenGL;

	RenderAPI*& RenderAPI::Get()
	{
		static RenderAPI* instance = nullptr;
		if (!instance)
		{
			switch (GetAPIType())
			{
			case API::OpenGL:
				instance = new OpenGL::OpenGLRenderAPI();
				break;
			default:
				instance = nullptr;
				break;
			}
		}
		return instance;
	}
}