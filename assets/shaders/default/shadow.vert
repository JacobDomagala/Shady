#version 460

layout(location = 0) in vec4 inPos;

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
   gl_Position = ubo.u_lightMat * bufferData.modelMat * inPos;
}
