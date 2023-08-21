#pragma once
#include "defines.h"
#include "glm/glm.hpp"
#include "containers/darray.h"
#include "core/gr_memory.h"

namespace GR
{
	struct Vertex
	{
		glm::vec2 position;
		glm::vec3 color;
	};
	
	struct VertexBuffer
	{
		void* internalState;
	};

	struct IndexBuffer
	{
		void* internalState;
	};
	
	VertexBuffer* CreateVertexBuffer(void* vertices, size_t size);
	void DestroyVertexBuffer(VertexBuffer* clientBuffer);

	/// <summary>
	/// Creates an index buffer
	/// </summary>
	/// <param name="indices">Array of indices, type needs to be u32</param>
	/// <param name="indexCount"></param>
	/// <returns></returns>
	IndexBuffer* CreateIndexBuffer(u32* indices, size_t indexCount);
	void DestroyIndexBuffer(IndexBuffer* clientBuffer);
}