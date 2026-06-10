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
   ivec4 textureIDs;
   vec4 baseColorFactor;
   vec4 materialFactors;
};

layout(std430, set = 0, binding = 1) readonly buffer Block
{
   BufferData Transforms[];
};

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in vec4 a_tangent;

layout(location = 0) out VS_OUT
{
   vec3 fPosition;
   vec2 fTexCoord;
   vec3 fNorm;
   vec4 fTangent;

   flat int fBaseColorSampl;
   flat int fMaterialSampl;
   flat int fNormSampl;
   flat vec4 fBaseColorFactor;
   flat vec3 fMaterialFactors;
}
vs_out;

void
main()
{
   BufferData bufferData = Transforms[gl_InstanceIndex];
   mat4 modelMat = bufferData.modelMat;

   gl_Position = ubo.u_viewProjectionMat * modelMat * vec4(a_position, 1.0);

   vs_out.fPosition = vec3(modelMat * vec4(a_position, 1.0));
   vs_out.fTexCoord = a_texCoord;

   mat3 normalMatrix = transpose(inverse(mat3(modelMat)));
   float orientation = determinant(mat3(modelMat)) < 0.0 ? -1.0 : 1.0;
   vs_out.fNorm = normalMatrix * normalize(a_normal);
   vs_out.fTangent =
      vec4(mat3(modelMat) * normalize(a_tangent.xyz), a_tangent.w * orientation);

   vs_out.fBaseColorSampl = bufferData.textureIDs.x;
   vs_out.fMaterialSampl = bufferData.textureIDs.y;
   vs_out.fNormSampl = bufferData.textureIDs.z;
   vs_out.fBaseColorFactor = bufferData.baseColorFactor;
   vs_out.fMaterialFactors = bufferData.materialFactors.xyz;
}
