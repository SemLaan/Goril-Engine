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

		VertexBufferElement(ShaderDataType dataType, size_t offset)
			: type(dataType), offset(offset) {}
	};

	// Defines how the GPU should interpret the data in a vertex buffer
	struct VertexBufferLayout
	{
		// All the elements that make up 1 vertex, for example [Vec2F, Mat4]
		std::vector<VertexBufferElement> elements;
		size_t stride;								// Total size of one vertex in bytes
		unsigned int instanced;							// Should this vertex buffer be instanced (0 no, 1 yes)

		VertexBufferLayout() = default;
		/// <summary>
		/// Creates a vertex buffer layout.
		/// </summary>
		/// <param name="layout">Data types that make up a vertex.</param>
		/// <param name="instanced">Whether this vertex buffer is instanced or not (0 no, 1 yes).</param>
		VertexBufferLayout(std::initializer_list<ShaderDataType> layout, unsigned int instanced = 0);
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		/// <summary>
		/// Sets new data in the buffer.
		/// </summary>
		/// <param name="pData">Pointer to data that will be put in the buffer.</param>
		/// <param name="insertRange">Where the data will be placed in the buffer, interpreted in bytes. (Be carefull to not exceed the buffer size)</param>
		virtual void SetBufferData(const void* pData, Range insertRange) = 0;

		virtual inline const VertexBufferLayout& GetBufferLayout() const = 0;
		virtual void SetBufferLayout(const VertexBufferLayout& layout) = 0;

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