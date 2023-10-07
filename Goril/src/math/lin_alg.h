#pragma once
#include "math_types.h"

#include "core/asserts.h"
// TODO: make it so this doesn't have to be included everywhere, because it is big and it's only needed for trig funcs
#include <math.h>

#define PI 3.14159265358979323846f


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

	mat.values[0][0] = 1.f;
	mat.values[1][1] = 1.f;
	mat.values[2][2] = 1.f;
	mat.values[3][3] = 1.f;

	return mat;
}

static mat4 mat4_scale(vec3 vec)
{
	mat4 mat = {};

	mat.values[0][0] = vec.x;
	mat.values[1][1] = vec.y;
	mat.values[2][2] = vec.z;
	mat.values[3][3] = 1.f;

	return mat;
}

static mat4 mat4_translate(vec3 vec)
{
	mat4 mat = {};

	mat.values[0][0] = 1.f;
	mat.values[1][1] = 1.f;
	mat.values[2][2] = 1.f;
	mat.values[3][3] = 1.f;

	mat.values[3][0] = vec.x;
	mat.values[3][1] = vec.y;
	mat.values[3][2] = vec.z;

	return mat;
}

static mat4 mat4_mul_mat4(mat4 a, mat4 b)
{
	mat4 result = {};

	for (u32 i = 0; i < 4; i++)
	{
		for (u32 j = 0; j < 4; j++)
		{
			result.values[j][i] = a.values[0][i] * b.values[j][0] + a.values[1][i] * b.values[j][1] + a.values[2][i] * b.values[j][2] + a.values[3][i] * b.values[j][3];
		}
	}

	return result;
}

static mat4 mat4_rotate_x(f32 angle_radians) 
{
	f32 c = (f32)cos(angle_radians);
	f32 s = (f32)sin(angle_radians);

	mat4 out_matrix = mat4_identity();
	out_matrix.values[1][1] = c;
	out_matrix.values[2][1] = s;
	out_matrix.values[1][2] = -s;
	out_matrix.values[2][2] = c;
	return out_matrix;
}

static mat4 mat4_rotate_y(f32 angle_radians) 
{
	f32 c = (f32)cos(angle_radians);
	f32 s = (f32)sin(angle_radians);

	mat4 out_matrix = mat4_identity();
	out_matrix.values[0][0] = c;
	out_matrix.values[2][0] = -s;
	out_matrix.values[0][2] = s;
	out_matrix.values[2][2] = c;
	return out_matrix;
}

static mat4 mat4_rotate_z(f32 angle_radians) 
{
	f32 c = (f32)cos(angle_radians);
	f32 s = (f32)sin(angle_radians);

	mat4 out_matrix = mat4_identity();
	out_matrix.values[0][0] = c;
	out_matrix.values[1][0] = s;
	out_matrix.values[0][1] = -s;
	out_matrix.values[1][1] = c;
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
			transposed.values[i][j] = mat.values[j][i];
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

	projection.values[0][0] = x;
	projection.values[1][1] = y;
	projection.values[2][2] = A;
	projection.values[3][2] = B;
	projection.values[3][3] = 0.f;
	projection.values[2][3] = -1.f;

	return projection;
}

static mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
	mat4 projection = {};

	projection.values[0][0] = 2.f / (right - left);
	projection.values[1][1] = 2.f / (bottom - top);
	projection.values[2][2] = -1.f / (far - near);
	projection.values[3][3] = 1.f;
	projection.values[0][3] = -(right + left) / (right - left);
	projection.values[1][3] = -(top + bottom) / (bottom - top);
	projection.values[2][3] = -(near) / (far - near);

	return projection;
}