#include "indexbuffer.h"
#include "rendererapi.h"
#include "opengl/openglindexbuffer.h"

namespace Goril::LLR
{
    Ref<IndexBuffer> IndexBuffer::Create(unsigned int count)
    {
        switch (RendererAPI::GetAPIType())
        {
        case API::OpenGL:
            return CreateRef<OpenGLIndexBuffer>(count);
        default:
            return nullptr;
        }
    }

    Ref<IndexBuffer> IndexBuffer::Create(const unsigned int* pData, unsigned int count)
    {
        switch (RendererAPI::GetAPIType())
        {
        case API::OpenGL:
            return CreateRef<OpenGLIndexBuffer>(pData, count);
        default:
            return nullptr;
        }
    }
}
