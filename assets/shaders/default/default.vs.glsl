#version 420 core

uniform mat4 u_viewProjection;

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in float a_DiffTexIndex;
layout (location = 5) in float a_NormTexIndex;
layout (location = 6) in float a_SpecTexIndex;
layout (location = 7) in mat4 a_ModelMat;
layout (location = 8) in vec4 a_Color;

out VS_OUT
{
    vec2 tc;
    flat int diffTex;
    flat int specTex;
    flat int normTex;
} vs_out;

void main(void)
{
    vs_out.tc = a_TexCoord;
    vs_out.diffTex = int(a_DiffTexIndex);
    vs_out.specTex = int(a_SpecTexIndex);
    vs_out.normTex = int(a_NormTexIndex);

    gl_Position = u_viewProjection  * vec4(a_Position, 1.0f);
}
