#pragma once
#include "defines.h"

typedef struct vec2i
{
	i32 x;
	i32 y;
} vec2i;

typedef struct vec2
{
	union {
		f32 x;
		//float u;
	};
	union {
		f32 y;
		//float v;
	};
} vec2;

typedef struct vec3
{
	union {
		f32 x;
		//f32 u;
		//f32 r;
	};
	union {
		f32 y;
		//f32 v;
		//f32 g;
	};
	union {
		f32 z;
		//f32 w;
		//f32 b;
	};
} vec3;

typedef struct vec4
{
	union {
		f32 x;
	};
	union {
		f32 y;
	};
	union {
		f32 z;
	};
	union {
		f32 w;
	};
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