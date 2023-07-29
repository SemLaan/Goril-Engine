#pragma once
#include "core/gorilmem.h"
#include "llrenums.h"

namespace Goril::LLR
{

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&&) = delete;
		IndexBuffer(IndexBuffer&&) = delete;

		/// <summary>
		/// Sets new data in the buffer.
		/// </summary>
		/// <param name="pData">Pointer to data that will be put in the buffer.</param>
		/// <param name="insertRange">Where the data will be placed in the buffer, interpreted in indices. (Be carefull to not exceed the buffer size)</param>
		virtual void SetBufferData(const unsigned int* pData, Range insertRange) = 0;

		/// <returns>How many indices this buffer has space for.</returns>
		virtual inline unsigned int GetCount() const = 0;

		/// <summary>
		/// Creates an index buffer on the gpu.
		/// </summary>
		/// <param name="count">Size of the buffer in amount of indices.</param>
		/// <returns>A smart pointer to the index buffer object</returns>
		static Ref<IndexBuffer> Create(unsigned int count);

		/// <summary>
		/// Creates an index buffer on the gpu.
		/// </summary>
		/// <param name="pData">Pointer to data that will be put into the buffer.</param>
		/// <param name="count">Amount of indices in data.</param>
		/// <returns>A smart pointer to the index buffer object</returns>
		static Ref<IndexBuffer> Create(const unsigned int* pData, unsigned int count);
	};
}