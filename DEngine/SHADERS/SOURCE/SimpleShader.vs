#version 430 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT{
	vec3 fPosition;
	vec3 fNormal;
	vec2 fTexCoord;
} vs_out;


void main()
{
    gl_Position = projection * view * model * vec4(vPosition, 1.0f);
	
	vs_out.fPosition = vec3(model * vec4(vPosition, 1.0f));
    vs_out.fTexCoord = vTexCoord;
	vs_out.fNormal = mat3(model) * vNormal;
}