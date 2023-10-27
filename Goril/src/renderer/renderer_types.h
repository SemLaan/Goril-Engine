#pragma once
#include "defines.h"
#include "math/math_types.h"
#include "containers/darray.h"

#define TEXTURE_CHANNELS 4


typedef struct Vertex
{
	vec3 position;
	vec3 color;
	vec2 texCoord;
} Vertex;

typedef struct SpriteInstance
{
	mat4 model;
	u32 textureIndex;
} SpriteInstance;

// Handle to a vertex buffer
typedef struct VertexBuffer
{
	void* internalState;
} VertexBuffer;

// Handle to an index buffer
typedef struct IndexBuffer
{
	void* internalState;
} IndexBuffer;

// Handle to a texture
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
	mat4 model;
	Texture texture;
} SpriteRenderInfo;

typedef struct SceneRenderData2D
{
	SpriteRenderInfo* spriteRenderInfoDarray;
	mat4 camera;
} SceneRenderData2D;