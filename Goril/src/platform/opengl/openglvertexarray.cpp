#include "openglvertexarray.h"
#include "openglerrormanagement.h"
#include <glad/glad.h>
#include "openglindexbuffer.h"
#include "openglvertexbuffer.h"
#include "rendering/lowlevelrenderer/llrenums.h"
#include <core/core.h>

namespace Goril::LLR::OpenGL
{

	OpenGLVertexArray::OpenGLVertexArray()
		: m_vertexBufferIndex(0)
	{
		G(glGenVertexArrays(1, &m_rendererID));
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		G(glDeleteVertexArrays(1, &m_rendererID));
	}

	void OpenGLVertexArray::Bind() const
	{
		G(glBindVertexArray(m_rendererID));
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer> vertexBuffer)
	{
		m_vertexBuffers.push_back(vertexBuffer);
		Ref<OpenGLVertexBuffer> vb = std::dynamic_pointer_cast<OpenGLVertexBuffer>(vertexBuffer);

		Bind();
		vb->Bind();

		const VertexBufferLayout& layout = vertexBuffer->GetBufferLayout();
		for (const VertexBufferElement& element : layout.elements)
		{
			if (element.type == ShaderDataType::Mat2 || element.type == ShaderDataType::Mat3 || element.type == ShaderDataType::Mat4)
				AddMatrixComponent(element, layout.stride, layout.instanced);
			else
				AddComponent(element, layout.stride, layout.instanced);
		}
	}

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:    return GL_FLOAT;
		case ShaderDataType::Vec2F:   return GL_FLOAT;
		case ShaderDataType::Vec3F:   return GL_FLOAT;
		case ShaderDataType::Vec4F:   return GL_FLOAT;
		case ShaderDataType::Mat2:   return GL_FLOAT;
		case ShaderDataType::Mat3:     return GL_FLOAT;
		case ShaderDataType::Mat4:     return GL_FLOAT;
		case ShaderDataType::Int:      return GL_INT;
		case ShaderDataType::Vec2I:     return GL_INT;
		case ShaderDataType::Vec3I:     return GL_INT;
		case ShaderDataType::Vec4I:     return GL_INT;
		case ShaderDataType::Bool:     return GL_BOOL;
		}

		GRASSERT_MSG(false, "shader data type not implemented or doesn't exist.");

		return 0;
	}

	void OpenGLVertexArray::AddMatrixComponent(const VertexBufferElement& element, size_t stride, unsigned int instanced)
	{
		unsigned int count = ShaderDataTypeComponentCount(element.type);
		for (unsigned int i = 0; i < count; i++)
		{
			G(glEnableVertexAttribArray(m_vertexBufferIndex));
			G(glVertexAttribPointer(m_vertexBufferIndex, count,
				ShaderDataTypeToOpenGLBaseType(element.type), GL_FALSE,
				(GLsizei)stride, (const void*)(element.offset + sizeof(float) * count * i)));
			G(glVertexAttribDivisor(m_vertexBufferIndex, instanced));
			m_vertexBufferIndex++;
		}
	}

	void OpenGLVertexArray::AddComponent(const VertexBufferElement& element, size_t stride, unsigned int instanced)
	{
		G(glEnableVertexAttribArray(m_vertexBufferIndex));
		G(glVertexAttribPointer(m_vertexBufferIndex, (GLint)ShaderDataTypeComponentCount(element.type),
			ShaderDataTypeToOpenGLBaseType(element.type), GL_FALSE,
			(GLsizei)stride, (const void*)element.offset));
		G(glVertexAttribDivisor(m_vertexBufferIndex, instanced));
		m_vertexBufferIndex++;
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer> indexBuffer)
	{
		Bind();
		std::dynamic_pointer_cast<OpenGLIndexBuffer>(indexBuffer)->Bind();
		m_indexBuffer = indexBuffer;
	}
}