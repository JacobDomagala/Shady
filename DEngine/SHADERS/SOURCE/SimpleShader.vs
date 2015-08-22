#version 430 core

uniform mat4 MVP;
uniform mat4 modelMatrix;


layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;


out vec3 fPosition;
out vec3 fNormal;
out vec2 fUv;
void main()
{
	
	gl_Position = MVP*vec4(inPosition.x, inPosition.y , inPosition.z , 1.0);

	fPosition = vec3 (modelMatrix * vec4(inPosition, 1.0));
	fNormal = mat3(modelMatrix) * inNormal;
	fUv = inUv;
	
	
}