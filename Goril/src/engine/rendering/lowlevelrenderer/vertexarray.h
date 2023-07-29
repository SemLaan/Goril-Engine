#pragma once
#include "llrenums.h"
#include "core/gorilmem.h"
#include "indexbuffer.h"
#include "vertexbuffer.h"

namespace Goril::LLR
{
	class VertexArray
	{
	protected:
		VertexArray() = default;
	public:
		virtual ~VertexArray() = default;
		VertexArray(const VertexArray&) = delete;
		VertexArray& operator=(const VertexArray&) = delete;
		VertexArray& operator=(VertexArray&&) = delete;
		VertexArray(VertexArray&&) = delete;

		virtual void AddVertexBuffer(const Ref<VertexBuffer> vertexBuffer) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer> indexBuffer) = 0;

		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;

		/// <summary>
		/// Creates a vertex array. The vertex array holds the vertex buffer(s) and index buffer.
		/// </summary>
		/// <returns>A smart pointer to the vertex array object.</returns>
		static Ref<VertexArray> Create();
	};
}