#pragma once

#define BIT(x) 1U << (x-1)


namespace Goril::LLR
{

	// Enum for specifying what buffers need to be cleared
	enum ClearOption
	{
		COLOR_BUFFER = BIT(1),
		DEPTH_BUFFER = BIT(2),
		STENCIL_BUFFER = BIT(3),
	};

	// Data types that shaders can understand
	enum class ShaderDataType
	{
		None = 0, Float, Vec2F, Vec3F, Vec4F, Mat3, Mat4, Int, Vec2I, Vec3I, Vec4I, Bool
	};

	// Byte sizes of all shader data types
	static size_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:    return 4;
		case ShaderDataType::Vec2F:   return 4 * 2;
		case ShaderDataType::Vec3F:   return 4 * 3;
		case ShaderDataType::Vec4F:   return 4 * 4;
		case ShaderDataType::Mat3:     return 4 * 3 * 3;
		case ShaderDataType::Mat4:     return 4 * 4 * 4;
		case ShaderDataType::Int:      return 4;
		case ShaderDataType::Vec2I:     return 4 * 2;
		case ShaderDataType::Vec3I:     return 4 * 3;
		case ShaderDataType::Vec4I:     return 4 * 4;
		case ShaderDataType::Bool:     return 1;
		}

		return 0;
	}

	static unsigned int ShaderDataTypeComponentCount(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:   return 1;
		case ShaderDataType::Vec2F:  return 2;
		case ShaderDataType::Vec3F:  return 3;
		case ShaderDataType::Vec4F:  return 4;
		case ShaderDataType::Mat3:    return 3; // 3* float3
		case ShaderDataType::Mat4:    return 4; // 4* float4
		case ShaderDataType::Int:     return 1;
		case ShaderDataType::Vec2I:    return 2;
		case ShaderDataType::Vec3I:    return 3;
		case ShaderDataType::Vec4I:    return 4;
		case ShaderDataType::Bool:    return 1;
		}
		return 0;
	}

	// Enum of all possible frame buffer texture formats
	enum class FramebufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		R8,

		// Depth/stencil
		DEPTH24STENCIL8,
		DEPTH = DEPTH24STENCIL8
	};

	// Enum of "all" possible texture formats
	enum class TextureFormat
	{
		None = 0, RGBA8, R8,
	};

	// Options for setting the blend state of the render pipeline
	enum class BlendOption
	{
		ZERO,
		ONE,
		SRC_COLOR,
		ONE_MINUS_SRC_COLOR,
		DST_COLOR,
		ONE_MINUS_DST_COLOR,
		SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA,
		DST_ALPHA,
		ONE_MINUS_DST_ALPHA,
		CONSTANT_COLOR,
		ONE_MINUS_CONSTANT_COLOR,
		CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA
	};

	// Options for setting the stencil state of the render pipeline
	enum class StencilOption
	{
		NEVER,
		INCR,
		KEEP,
		DECR,
		EQUAL,
		ALWAYS,
	};

	// All implemented rendering API's
	enum class API
	{
		None = 0, OpenGL = 1
	};

	struct Range
	{
		size_t start;
		size_t end;
	};
}