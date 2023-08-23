#pragma once
#include "defines.h"
#include "glm/glm.hpp"
#include "containers/darray.h"


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

	struct GlobalUniformObject
	{
		glm::mat4 projView;
	};
}