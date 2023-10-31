#pragma once
#include "math_types.h"

#include "core/asserts.h"
#include <immintrin.h>
// TODO: make it so this doesn't have to be included everywhere, because it is big and it's only needed for trig funcs
#include <math.h>


// !!!!!!!!!!!!!!! Matrices are stored column-wise ==========================================================

#define PI 3.14159265358979323846f
#define COL4(col_z) (col_z * 4)
#define COL3(col_z) (col_z * 3)
#define COL2(col_z) (col_z * 2)

// a b
// c d
#define M2a 0
#define M2b 2
#define M2c 1
#define M2d 3

// a b c
// d e f
// g h i
#define M3a 0
#define M3d 1
#define M3g 2
#define M3b 3
#define M3e 4
#define M3h 5
#define M3c 6
#define M3f 7
#define M3i 8



static vec3 vec3_from_float(f32 value)
{
	return (vec3){ value, value, value };
}

static vec3 vec3_add_vec3(vec3 v1, vec3 v2)
{
	return (vec3){ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

static vec3 vec3_min_vec3(vec3 v1, vec3 v2)
{
	return (vec3){ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

static vec3 vec3_div_float(vec3 vec, f32 value)
{
	return (vec3){ vec.x / value, vec.y / value, vec.z / value };
}

static f32 vec4_dot(vec4 v1, vec4 v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

static mat4 mat4_identity()
{
	mat4 mat = {};

	mat.values[0] = 1.f;
	mat.values[1 + COL4(1)] = 1.f;
	mat.values[2 + COL4(2)] = 1.f;
	mat.values[3 + COL4(3)] = 1.f;

	return mat;
}

static mat4 mat4_scale(vec3 vec)
{
	mat4 mat = {};

	mat.values[0] = vec.x;
	mat.values[1 + COL4(1)] = vec.y;
	mat.values[2 + COL4(2)] = vec.z;
	mat.values[3 + COL4(3)] = 1.f;

	return mat;
}

static mat4 mat4_translate(vec3 vec)
{
	mat4 mat = {};

	mat.values[0] = 1.f;
	mat.values[1 + COL4(1)] = 1.f;
	mat.values[2 + COL4(2)] = 1.f;
	mat.values[3 + COL4(3)] = 1.f;

	mat.values[0 + COL4(3)] = vec.x;
	mat.values[1 + COL4(3)] = vec.y;
	mat.values[2 + COL4(3)] = vec.z;

	return mat;
}

static mat4 mat4_mul_mat4(mat4 a, mat4 b)
{
	mat4 result = {};

	for (u32 i = 0; i < 4; i++)
	{
		for (u32 j = 0; j < 4; j++)
		{
			result.values[i + COL4(j)] = a.values[i + COL4(0)] * b.values[0 + COL4(j)] + a.values[i + COL4(1)] * b.values[1 + COL4(j)] + a.values[i + COL4(2)] * b.values[2 + COL4(j)] + a.values[i + COL4(3)] * b.values[3 + COL4(j)];
		}
	}

	return result;
}

static mat4 mat4_rotate_x(f32 angle_radians) 
{
	f32 c = (f32)cos(angle_radians);
	f32 s = (f32)sin(angle_radians);

	mat4 out_matrix = mat4_identity();
	out_matrix.values[1 + COL4(1)] = c;
	out_matrix.values[1 + COL4(2)] = s;
	out_matrix.values[2 + COL4(1)] = -s;
	out_matrix.values[2 + COL4(2)] = c;
	return out_matrix;
}

static mat4 mat4_rotate_y(f32 angle_radians) 
{
	f32 c = (f32)cos(angle_radians);
	f32 s = (f32)sin(angle_radians);

	mat4 out_matrix = mat4_identity();
	out_matrix.values[0 + COL4(0)] = c;
	out_matrix.values[0 + COL4(2)] = -s;
	out_matrix.values[2 + COL4(0)] = s;
	out_matrix.values[2 + COL4(2)] = c;
	return out_matrix;
}

static mat4 mat4_rotate_z(f32 angle_radians) 
{
	f32 c = (f32)cos(angle_radians);
	f32 s = (f32)sin(angle_radians);

	mat4 out_matrix = mat4_identity();
	out_matrix.values[0 + COL4(0)] = c;
	out_matrix.values[0 + COL4(1)] = s;
	out_matrix.values[1 + COL4(0)] = -s;
	out_matrix.values[1 + COL4(1)] = c;
	return out_matrix;
}

static mat4 mat4_rotate_xyz(vec3 angles)
{
	mat4 rx = mat4_rotate_x(angles.x);
	mat4 ry = mat4_rotate_y(angles.y);
	mat4 rz = mat4_rotate_z(angles.z);
	mat4 out_matrix = mat4_mul_mat4(rx, ry);
	out_matrix = mat4_mul_mat4(out_matrix, rz);
	return out_matrix;
}

static mat4 mat4_transpose(mat4 mat)
{
	mat4 transposed = mat;

	for (u32 i = 0; i < 4; ++i)
	{
		for (u32 j = 0; j < 4; ++j)
		{
			transposed.values[j + COL4(i)] = mat.values[i + COL4(j)];
		}
	}

	return transposed;
}

static mat4 mat4_perspective(f32 verticalFovDegrees, f32 aspectRatio, f32 near, f32 far)
{
	f32 verticalFovRadians = verticalFovDegrees * 2.0f * PI / 360.0f;
	f32 focalLength = 1.0f / (f32)tan(verticalFovRadians / 2.0f);

	f32 x = focalLength / aspectRatio;
	f32 y = -focalLength;
	f32 A = near / (far - near);
	f32 B = far * A;

	mat4 projection = {};

	projection.values[0 + COL4(0)] = x;
	projection.values[1 + COL4(1)] = y;
	projection.values[2 + COL4(2)] = A;
	projection.values[2 + COL4(3)] = B;
	projection.values[3 + COL4(3)] = 0.f;
	projection.values[3 + COL4(2)] = -1.f;

	return projection;
}

static mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
	mat4 projection = {};

	projection.values[0 + COL4(0)] = 2.f / (right - left);
	projection.values[1 + COL4(1)] = 2.f / (bottom - top);
	projection.values[2 + COL4(2)] = -1.f / (far - near);
	projection.values[3 + COL4(3)] = 1.f;
	projection.values[3 + COL4(0)] = -(right + left) / (right - left);
	projection.values[3 + COL4(1)] = -(top + bottom) / (bottom - top);
	projection.values[3 + COL4(2)] = -(near) / (far - near);

	return projection;
}
