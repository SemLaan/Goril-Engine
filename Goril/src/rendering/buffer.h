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
	
	VertexBuffer* CreateVertexBuffer();

	void DestroyVertexBuffer(VertexBuffer* clientBuffer);
}