#version 460

layout(location = 0) in vec4 inPos;
layout(location = 2) in vec2 inUV;
layout(location = 6) in vec4 inColor;
layout(location = 0) out vec2 outUV;
layout(location = 1) out float outAlpha;
layout(location = 2) flat out float outCutoff;
layout(location = 3) flat out int outTexture;

layout(binding = 0) uniform UBO
{
   mat4 u_projectionMat;
   mat4 u_viewMat;
   mat4 u_lightMat;
}
ubo;

struct BufferData
{
   mat4 modelMat;
   ivec4 textureIDs;
   vec4 baseColorFactor;
   vec4 materialFactors;
};

layout(std430, set = 0, binding = 1) readonly buffer Block
{
   BufferData Transforms[];
};

void
main()
{
   BufferData bufferData = Transforms[gl_InstanceIndex];
   outUV = inUV;
   outAlpha = inColor.a * bufferData.baseColorFactor.a;
   outCutoff = bufferData.materialFactors.w;
   outTexture = bufferData.textureIDs.x;
   gl_Position = ubo.u_lightMat * bufferData.modelMat * inPos;
}
