#version 430 core

layout (location = 0) in vec3 a_Position;

out vec3 TexCoords;

uniform mat4 u_viewProjection;

void main()
{
    TexCoords = a_Position;
    gl_Position = u_viewProjection * vec4(a_Position, 1.0);
}