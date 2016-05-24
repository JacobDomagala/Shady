#version 420 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;

uniform vec3 vCameraPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform float time;

out VS_OUT{
	vec3 fPosition;
	vec3 fNormal;
	vec2 fTexCoord;
	vec3 fLightPosition;
	vec3 fCameraPosition;
	mat3 TBN;
} vs_out;


void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vPosition, 1.0f);
	
	vec3 N = normalize(mat3(modelMatrix) * vNormal);
	vec3 T = normalize(mat3(modelMatrix) * vTangent);
	vec3 B = normalize(cross(N,T));
	vs_out.TBN = mat3(T,B,N);

	vs_out.fPosition = vec3(modelMatrix * vec4(vPosition, 1.0f));
    vs_out.fTexCoord = vTexCoord;
	
	vs_out.fLightPosition = vec3(0.0f, sin(time)*1.5f, 0.0f);
	vs_out.fCameraPosition = vCameraPosition;
}