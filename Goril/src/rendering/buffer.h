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
	
	GRAPI VertexBuffer* CreateVertexBuffer(void* vertices, size_t size);
	GRAPI void DestroyVertexBuffer(VertexBuffer* clientBuffer);

	/// <summary>
	/// Creates an index buffer
	/// </summary>
	/// <param name="indices">Array of indices, type needs to be u32</param>
	/// <param name="indexCount"></param>
	/// <returns></returns>
	GRAPI IndexBuffer* CreateIndexBuffer(u32* indices, size_t indexCount);
	GRAPI void DestroyIndexBuffer(IndexBuffer* clientBuffer);
}