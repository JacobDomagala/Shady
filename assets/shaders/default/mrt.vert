#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_draw_parameters : require

layout(set = 0, binding = 0) uniform UniformBufferObject
{
   mat4 u_viewProjectionMat;
   mat4 u_lightMat;
}
ubo;

struct BufferData
{
   mat4 modelMat;
   vec4 textureIDs;
};

layout(std430, set = 0, binding = 1) readonly buffer Block
{
   BufferData Transforms[];
};

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in vec3 a_tangent;

layout(location = 0) out VS_OUT
{
   vec3 fPosition;
   vec2 fTexCoord;
   vec3 fNorm;
   vec3 fTangent;

   flat int fDiffSampl;
   flat int fNormSampl;
   flat int fSpecSampl;
}
vs_out;

void
main()
{
   BufferData bufferData = Transforms[gl_InstanceIndex];
   mat4 modelMat = bufferData.modelMat;

   gl_Position = ubo.u_viewProjectionMat * modelMat * vec4(a_position, 1.0f);

   vs_out.fTexCoord = a_texCoord;

   // Vertex position in world space
   vs_out.fPosition = vec3(modelMat * vec4(a_position, 1.0f));

   // Normal in world space
   mat3 mNormal = transpose(inverse(mat3(modelMat)));
   vs_out.fNorm = mNormal * normalize(a_normal);
   vs_out.fTangent = mNormal * normalize(a_tangent);
   vs_out.fDiffSampl = int(bufferData.textureIDs.x);
   vs_out.fNormSampl = int(bufferData.textureIDs.y);
   vs_out.fSpecSampl = int(bufferData.textureIDs.z);
}
