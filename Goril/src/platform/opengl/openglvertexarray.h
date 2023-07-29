#pragma once
#include "rendering/lowlevelrenderer/vertexarray.h"


namespace Goril::LLR::OpenGL
{

	class OpenGLVertexArray : public VertexArray
	{
	private:
		unsigned int m_rendererID;
		unsigned int m_vertexBufferIndex;
		Ref<IndexBuffer> m_indexBuffer;
		std::vector<Ref<VertexBuffer>> m_vertexBuffers;
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray();

		void Bind() const;

		// Inherited via VertexArray
		void AddVertexBuffer(const Ref<VertexBuffer> vertexBuffer) override;
		void SetIndexBuffer(const Ref<IndexBuffer> indexBuffer) override;
		const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return m_vertexBuffers; }
		const Ref<IndexBuffer>& GetIndexBuffer() const override { return m_indexBuffer; }
	private:
		void AddMatrixComponent(const VertexBufferElement& element, size_t stride, unsigned int instanced);
		void AddComponent(const VertexBufferElement& element, size_t stride, unsigned int instanced);
	};
}