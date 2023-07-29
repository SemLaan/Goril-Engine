#include "vertexarray.h"
#include "rendererapi.h"
#include "opengl/openglvertexarray.h"

namespace Goril::LLR
{
	Ref<VertexArray> VertexArray::Create()
	{
        switch (RendererAPI::GetAPIType())
        {
        case API::OpenGL:
            return CreateRef<OpenGL::OpenGLVertexArray>();
        default:
            return nullptr;
        }
	}
}