#pragma once
#include "rendering/lowlevelrenderer/vertexbuffer.h"

namespace Goril::LLR::OpenGL
{

	class OpenGLVertexBuffer : public VertexBuffer
	{
	private:
		unsigned int m_rendererID;		// Just an id for the buffer
		VertexBufferLayout m_layout;	// Layout of the buffer
		size_t m_size;					// Size of the buffer in bytes
	public:
		OpenGLVertexBuffer(size_t size);
		OpenGLVertexBuffer(const void* pData, size_t size);

		void Bind() const;

		void SetBufferData(const void* pData, Range insertRange) override;
		inline size_t GetBufferSize() const override { return m_size; }

		virtual inline const VertexBufferLayout& GetBufferLayout() const override { return m_layout; }
		virtual void SetBufferLayout(const VertexBufferLayout& layout) override { m_layout = layout; }
	};
}