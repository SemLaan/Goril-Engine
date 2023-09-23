#version 450

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_color;
layout(location = 2) in vec2 v_texCoord;
layout(location = 3) in mat4 v_model;
layout(location = 7) in uint v_textureIndex;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 texCoord;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 projView;
} ubo;


void main() {
	fragColor = v_color;
	texCoord = v_texCoord;
	gl_Position = ubo.projView * v_model * vec4(v_position, 1.0);
}