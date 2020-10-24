#version 420 core

uniform sampler2D u_Textures[32];

in VS_OUT
{
    vec2 tc;
    flat int diffTex;
    flat int specTex;
    flat int normTex;
} fs_in;

out vec4 color;

void main(void)
{
    color =  texture(u_Textures[fs_in.diffTex], fs_in.tc);
}
