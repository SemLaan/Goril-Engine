#include "openglvertexbuffer.h"
#include "openglerrormanagement.h"
#include <glad/glad.h>

namespace Goril::LLR::OpenGL
{

	OpenGLVertexBuffer::OpenGLVertexBuffer(const void* pData, size_t size)
		: m_size(size)
	{
		G(glGenBuffers(1, &m_rendererID));
		Bind();
		G(glBufferData(GL_ARRAY_BUFFER, size, pData, GL_STATIC_DRAW));
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(size_t size)
		: m_size(size)
	{
		G(glGenBuffers(1, &m_rendererID));
		Bind();
		G(glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));
	}

	void OpenGLVertexBuffer::SetBufferData(const void* pData, Range insertRange)
	{
		Bind();
		G(glBufferSubData(GL_ARRAY_BUFFER, insertRange.start, insertRange.end - insertRange.start, pData));
	}

	void OpenGLVertexBuffer::Bind() const
	{
		G(glBindBuffer(GL_ARRAY_BUFFER, m_rendererID));
	}
}