#version 410 core

uniform sampler2D sampler;

uniform vec3 lightPosition;
uniform float time;

in VS_OUT
{
    vec3 normal;
    vec3 view;
} fs_in;

out vec4 outputColor;

void main(void)
{
    // Calculate per-pixel normal and light vector
    vec3 N = normalize(fs_in.normal);
    vec3 L = normalize(lightPosition - fs_in.view);

    // Simple N dot L diffuse lighting
    float tc = pow(max(0.0, dot(N, L)), 5.0);
	vec2 k = vec2(tc, tc) * sin(time*0.5f);
    // Sample from cell shading texture
    outputColor = texture2D(sampler, k) * (tc * 0.8 + 0.2);
}
