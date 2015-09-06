#version 430 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
layout (location = 3) in vec3 vTangent;
layout (location = 4) in vec3 vBTangent;

uniform vec3 vLightPosition;
uniform vec3 vCameraPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT{
	vec3 fPosition;
	vec2 fTexCoord;
	vec3 fCameraPosition;
	vec3 fLightPosition;
} vs_out;


void main()
{
    gl_Position = projection * view * model * vec4(vPosition, 1.0f);
	
	
	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vec3 T = normalize(normalMatrix * vTangent);
	vec3 B = normalize(normalMatrix * vBTangent);
	vec3 N = normalize(normalMatrix * vNormal);
	mat3 TBN = transpose(mat3(T,B,N));
	
	vs_out.fPosition = TBN * vec3(model * vec4(vPosition, 1.0f));
	vs_out.fTexCoord = vTexCoord;
	vs_out.fCameraPosition = TBN * vCameraPosition;
	vs_out.fLightPosition = TBN * vLightPosition;
	
	
	
}