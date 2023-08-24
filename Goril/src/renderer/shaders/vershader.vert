#version 450

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_color;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 projView;
} ubo;

layout(push_constant) uniform Push
{
	mat4 model;
} push;

void main() {
	fragColor = v_color;
	gl_Position = ubo.projView * push.model * vec4(v_position, 1.0);
}