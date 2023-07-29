#include "openglindexbuffer.h"
#include "openglerrormanagement.h"
#include "glad/glad.h"

namespace Goril::LLR::OpenGL
{

	OpenGLIndexBuffer::OpenGLIndexBuffer(unsigned int count)
		: m_count(count)
	{
		G(glGenBuffers(1, &m_rendererID));
		Bind();
		G(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW));
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(const unsigned int* pData, unsigned int count)
		: m_count(count)
	{
		G(glGenBuffers(1, &m_rendererID));
		Bind();
		G(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), pData, GL_STATIC_DRAW));
	}

	void OpenGLIndexBuffer::Bind() const
	{
		G(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID));
	}

	void OpenGLIndexBuffer::SetBufferData(const unsigned int* pData, Range insertRange)
	{
		Bind();
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, insertRange.start * sizeof(unsigned int), (insertRange.end - insertRange.start) * sizeof(unsigned int), pData);
	}
}