#pragma once
#include "core/gorilmem.h"
#include "llrenums.h"
#include <vector>

namespace Goril::LLR
{

	// Struct with all the information necessary to describe a single vertex attribute
	struct VertexBufferElement
	{
		ShaderDataType type;    // Datatype
		size_t offset;			// How many bytes from the start of the vertex to the start of this element

		VertexBufferElement(ShaderDataType dataType)
			: type(dataType), offset(ShaderDataTypeSize(type)) {}
	};

	// Defines how the GPU should interpret the data in a vertex buffer
	class VertexBufferLayout
	{
	private:
		// All the elements that make up 1 vertex, for example [Vec2F, Mat4]
		std::vector<VertexBufferElement> m_elements;
		size_t m_stride;								// Total size of one vertex in bytes
		unsigned int m_instanced;							// Should this vertex buffer be instanced (0 no, 1 yes)

	public:
		/// <summary>
		/// Creates a vertex buffer layout.
		/// </summary>
		/// <param name="layout">Data types that make up a vertex.</param>
		/// <param name="instanced">Whether this vertex buffer is instanced or not (0 no, 1 yes).</param>
		VertexBufferLayout(std::initializer_list<ShaderDataType> layout, unsigned int instanced = 0);

		/// <returns>The vector of elements that make up a vertex.</returns>
		inline const std::vector<VertexBufferElement>& GetElements() const { return m_elements; }
		/// <returns>Size of a vertex in bytes (aka stride).</returns>
		inline size_t GetStride() const { return m_stride; }
	};

	class VertexBuffer
	{
	private:
		VertexBufferLayout m_layout;

	public:
		virtual ~VertexBuffer() = default;

		/// <summary>
		/// Sets new data in the buffer.
		/// </summary>
		/// <param name="pData">Pointer to data that will be put in the buffer.</param>
		/// <param name="insertRange">Where the data will be placed in the buffer, interpreted in bytes. (Be carefull to not exceed the buffer size)</param>
		virtual void SetBufferData(const void* pData, Range insertRange) = 0;

		inline const VertexBufferLayout& GetBufferLayout() const { return m_layout; }
		void SetBufferLayout(const VertexBufferLayout& layout) { m_layout = layout; }

		/// <returns>Size of the buffer in bytes.</returns>
		virtual inline size_t GetBufferSize() const = 0;

		/// <summary>
		/// Creates a vertex buffer on the gpu.
		/// </summary>
		/// <param name="size">Size of the buffer in bytes.</param>
		/// <returns>A smart pointer to the vertex buffer object</returns>
		static Ref<VertexBuffer> Create(size_t size);

		/// <summary>
		/// Creates a vertex buffer on the gpu.
		/// </summary>
		/// <param name="pData">Pointer to data that will be put into the buffer.</param>
		/// <param name="size">Size of the data in bytes.</param>
		/// <returns>A smart pointer to the vertex buffer object</returns>
		static Ref<VertexBuffer> Create(const void* pData, size_t size);
	};
}