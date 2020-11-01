#version 460 core

uniform mat4 u_projectionMat;
uniform mat4 u_viewMat;

struct BufferData{
   mat4 modelMat;
   int diffuseMapID;
   int normalMapID;
   int specularMapID;
};

layout (std140, binding = 0) buffer CB0
{
    BufferData Transforms[];
};

uniform vec3 u_cameraPos;
uniform vec3 u_lightPos;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in vec3 a_tangent;
layout(location = 4) in vec4 a_color;
//layout(location = 5) in int a_drawID;

out VS_OUT
{
   vec3 fPosition;
   vec2 fTexCoord;
   vec3 fColor;
   vec3 fCameraPosition;
   vec3 fLightPosition;

   mat3 TBN;
   flat int fDiffTex;
   flat int fSpecTex;
   flat int fNormTex;
}
vs_out;

void
main(void)
{
   BufferData bufferData = Transforms[gl_DrawID];

   mat4 modelMat = bufferData.modelMat;

   mat3 normalMatrix = transpose(inverse(mat3(modelMat)));
   vec3 T = normalize(a_tangent);
   vec3 N = normalize(a_normal);
   T = normalize(T - (dot(T, N) * N));

   vec3 B = normalize(cross(T, N));
   vs_out.TBN = mat3(T, B, N);

   vs_out.fPosition = vec3(modelMat * vec4(a_position, 1.0f));
   vs_out.fTexCoord = a_texCoord;
   vs_out.fCameraPosition = u_cameraPos;
   vs_out.fLightPosition = u_lightPos;

   vs_out.fDiffTex = bufferData.diffuseMapID;
   vs_out.fNormTex = bufferData.normalMapID;
   vs_out.fSpecTex = bufferData.specularMapID;

   gl_Position = u_projectionMat * u_viewMat * modelMat * vec4(a_position, 1.0f);
}
