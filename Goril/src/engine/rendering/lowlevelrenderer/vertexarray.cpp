#include "vertexarray.h"
#include "renderapi.h"
#include "opengl/openglvertexarray.h"

namespace Goril::LLR
{
	Ref<VertexArray> VertexArray::Create()
	{
        switch (RenderAPI::GetAPIType())
        {
        case API::OpenGL:
            return CreateRef<OpenGL::OpenGLVertexArray>();
        default:
            return nullptr;
        }
	}
}