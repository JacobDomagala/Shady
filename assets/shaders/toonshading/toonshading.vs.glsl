#version 410 core

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

uniform mat4 MVP;
uniform mat4 modelMatrix;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;

out VS_OUT
{
    vec3 normal;
    vec3 view;
} vs_out;

void main(void)
{
    vec4 pos_vs = modelMatrix * vec4(inPosition, 1.0);

    // Calculate eye-space normal and position
    vs_out.normal = mat3(modelMatrix) * inNormal;
    vs_out.view = pos_vs.xyz;

    // Send clip-space position to primitive assembly
    gl_Position = MVP * vec4(inPosition, 1.0);
}
