#version 430 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;

uniform vec3 vLightPosition;
uniform vec3 vCameraPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 lightSpaceMatrix;

out VS_OUT{
	vec3 fPosition;
	vec3 fNormal;
	vec2 fTexCoord;
	vec4 fLightSpacePosition;
	vec3 fLightPosition;
	vec3 fCameraPosition;
} vs_out;


void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vPosition, 1.0f);
	
	vs_out.fPosition = vec3(modelMatrix * vec4(vPosition, 1.0f));
    vs_out.fTexCoord = vTexCoord;
	vs_out.fNormal = mat3(modelMatrix) * vNormal;
	vs_out.fLightSpacePosition = lightSpaceMatrix * vec4(vs_out.fPosition, 1.0f);
	vs_out.fLightPosition = vLightPosition;
	vs_out.fCameraPosition = vCameraPosition;
}