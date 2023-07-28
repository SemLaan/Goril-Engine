#include "rendererapi.h"

namespace Goril::LLR
{
	API RendererAPI::s_API = API::OpenGL;

	Scope<RendererAPI> RendererAPI::Create()
	{
		switch (GetAPI())
		{
		case API::OpenGL:
			return nullptr;
		default:
			return nullptr;
		}
	}
}