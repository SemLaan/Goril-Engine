#include "vertexbuffer.h"
#include "opengl/openglvertexbuffer.h"
#include "rendererapi.h"

namespace Goril::LLR
{
    Ref<VertexBuffer> VertexBuffer::Create(size_t size)
    {
        switch (RendererAPI::GetAPIType())
        {
        case API::OpenGL:
            return CreateRef<OpenGL::OpenGLVertexBuffer>(size);
        default:
            return nullptr;
        }
    }

    Ref<VertexBuffer> VertexBuffer::Create(const void* pData, size_t size)
    {
        switch (RendererAPI::GetAPIType())
        {
        case API::OpenGL:
            return CreateRef<OpenGL::OpenGLVertexBuffer>(pData, size);
        default:
            return nullptr;
        }
    }
}

