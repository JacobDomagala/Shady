#version 430 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;


uniform vec3 vLightPosition;
uniform vec3 vCameraPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 lightSpaceMatrix;

out VS_OUT{
	vec3 fPosition;
	vec2 fTexCoord;
	vec3 fCameraPosition;
	vec3 fLightPosition;
	mat3 TBN;
	vec4 fLightSpacePosition;
} vs_out;


void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vPosition, 1.0f);
	
	
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	vec3 T = normalize(normalMatrix * vTangent);
	vec3 N = normalize(normalMatrix * vNormal);
	T = normalize(T - (dot(T, N) * N));
	
	vec3 B = normalize(cross(T, N));
	vs_out.TBN = mat3(T,B,N);
	
	vs_out.fPosition = vec3(modelMatrix * vec4(vPosition, 1.0f));
	vs_out.fTexCoord = vTexCoord;
	vs_out.fCameraPosition = vCameraPosition;
	vs_out.fLightPosition = vLightPosition;
	vs_out.fLightSpacePosition = lightSpaceMatrix * vec4(vs_out.fPosition, 1.0f);
	
	
}