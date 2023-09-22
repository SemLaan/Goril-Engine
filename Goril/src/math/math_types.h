#pragma once
#include "defines.h"

typedef struct vec2i
{
	i32 x;
	i32 y;
} vec2i;

typedef struct vec2
{
	f32 x;
	f32 y;
} vec2;

typedef struct vec3
{
	f32 x;
	f32 y;
	f32 z;
} vec3;

typedef struct vec4
{
	f32 x;
	f32 y;
	f32 z;
	f32 w;
} vec4;

typedef struct mat2
{
	f32 values[2][2];
} mat2;

typedef struct mat3
{
	f32 values[3][3];
} mat3;

typedef struct mat4
{
	f32 values[4][4];
} mat4;