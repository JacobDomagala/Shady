#version 460
layout(constant_id = 0) const int NUM_TEXTURES = 256;
layout(binding = 2) uniform sampler samp;
layout(binding = 3) uniform texture2D textures[NUM_TEXTURES];
layout(location = 0) in vec2 inUV;
layout(location = 1) in float inAlpha;
layout(location = 2) flat in float inCutoff;
layout(location = 3) flat in int inTexture;
void main()
{
   if (inCutoff < 0.0) return;
   float alpha = inAlpha;
   if (inTexture >= 0) alpha *= texture(sampler2D(textures[inTexture], samp), inUV).a;
   if (alpha < inCutoff) discard;
}
