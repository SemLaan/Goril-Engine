#pragma once
#include "defines.h"
#include "math/math_types.h"
#include "containers/darray.h"


struct Vertex
{
	vec3 position;
	vec3 color;
	vec2 texCoord;
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
	mat4 projView;
};

struct PushConstantObject
{
	mat4 model;
};
