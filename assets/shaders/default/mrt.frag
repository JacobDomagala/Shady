#version 460

layout (constant_id = 0) const int NUM_TEXTURES = 256;

layout(binding = 2) uniform sampler samp;
layout(binding = 3) uniform texture2D textures[NUM_TEXTURES];


layout(location = 0) in VS_OUT
{
   vec3 fPosition;
   vec2 fTexCoord;
   vec3 fNorm;
   vec3 fTangent;

   flat int fDiffSampl;
   flat int fNormSampl;
   flat int fSpecSampl;
}
fs_in;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

void
main()
{
   outPosition = vec4(fs_in.fPosition, 1.0);

   // Calculate normal in tangent space
   vec3 N = normalize(fs_in.fNorm);
   vec3 T = normalize(fs_in.fTangent - N * dot(fs_in.fTangent, N));
   if (length(T) < 0.0001)
   {
      outNormal = vec4(N, 1.0);
   }
   else
   {
      vec3 B = normalize(cross(N, T));
      mat3 TBN = mat3(T, B, N);

   vec3 tnorm =
         TBN
         * normalize(texture(sampler2D(textures[fs_in.fNormSampl], samp), fs_in.fTexCoord).xyz * 2.0
                     - vec3(1.0));
      outNormal = vec4(tnorm, 1.0);
   }

   float specular = texture(sampler2D(textures[fs_in.fSpecSampl], samp), fs_in.fTexCoord).b;
   outAlbedo = vec4(texture(sampler2D(textures[fs_in.fDiffSampl], samp), fs_in.fTexCoord).rgb, specular);
}
