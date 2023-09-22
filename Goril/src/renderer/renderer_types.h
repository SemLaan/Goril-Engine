#pragma once
#include "defines.h"
#include "math/math_types.h"
#include "containers/darray.h"


typedef struct Vertex
{
	vec3 position;
	vec3 color;
	vec2 texCoord;
} Vertex;

typedef struct VertexBuffer
{
	void* internalState;
} VertexBuffer;

typedef struct IndexBuffer
{
	void* internalState;
} IndexBuffer;

typedef struct Texture
{
	void* internalState;
} Texture;

typedef struct GlobalUniformObject
{
	mat4 projView;
} GlobalUniformObject;

typedef struct PushConstantObject
{
	mat4 model;
} PushConstantObject;

typedef struct SpriteRenderInfo
{
	Texture texture;
	mat4 model;
} SpriteRenderInfo;

typedef struct SceneRenderData2D
{
	SpriteRenderInfo* spriteRenderInfoDarray;
	mat4 camera;
} SceneRenderData2D;