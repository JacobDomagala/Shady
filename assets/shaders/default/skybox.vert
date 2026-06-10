#version 460

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UBO
{
	mat4 view_projection;
} ubo;

layout (location = 0) out vec3 outUVW;

void main()
{
	outUVW = inPos;
	gl_Position = ubo.view_projection * vec4(inPos.xyz, 1.0);
}
