#pragma once
#include "defines.h"
#include "glm/glm.hpp"
#include "containers/darray.h"


namespace GR
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 texCoord;
	};

	struct VertexBuffer
	{
		void* internalState;
	};

	struct IndexBuffer
	{
		void* internalState;
	};

	struct Texture
	{
		void* internalState;
	};

	struct GlobalUniformObject
	{
		glm::mat4 projView;
	};

	struct PushConstantObject
	{
		glm::mat4 model;
	};
}