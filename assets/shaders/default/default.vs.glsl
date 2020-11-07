#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_draw_parameters : require
#extension GL_ARB_gpu_shader_int64 : enable

uniform mat4 u_projectionMat;
uniform mat4 u_viewMat;
uniform vec3 u_cameraPos;
uniform vec3 u_lightPos;

struct BufferData{
   mat4 modelMat;
   uint64_t diffuseMapID;
   uint64_t normalMapID;
   uint64_t specularMapID;
};

layout (std430, binding = 0) buffer CB0
{
    BufferData Transforms[];
};

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in vec3 a_tangent;
layout(location = 4) in vec4 a_color;

out VS_OUT
{
   vec3 fPosition;
   vec2 fTexCoord;
   vec3 fNorm;
   vec3 fColor;
   vec3 fCameraPosition;
   vec3 fLightPosition;

   mat3 TBN;
   flat uint64_t fDiffSampl;
   flat uint64_t fNormSampl;
   flat uint64_t fSpecSampl;
}
vs_out;

void
main(void)
{
   BufferData bufferData = Transforms[gl_DrawID];

   //mat4 modelMat = mat4(1.0);
   mat4 modelMat = bufferData.modelMat;

   mat3 normalMatrix = transpose(inverse(mat3(modelMat)));
   vec3 T = normalize(a_tangent);
   vec3 N = normalize(a_normal);
   T = normalize(T - (dot(T, N) * N));

   vec3 B = normalize(cross(T, N));
   vs_out.TBN = mat3(T, B, N);

   vs_out.fPosition = a_position;//vec3(modelMat * vec4(a_position, 1.0f));
   vs_out.fTexCoord = a_texCoord;
   vs_out.fCameraPosition = u_cameraPos;
   vs_out.fLightPosition = u_lightPos;

   vs_out.fDiffSampl = bufferData.diffuseMapID;
   vs_out.fNormSampl = bufferData.normalMapID;
   vs_out.fSpecSampl = bufferData.specularMapID;

   vs_out.fNorm = a_normal;

   gl_Position = u_projectionMat * u_viewMat * modelMat * vec4(a_position, 1.0f);
}
