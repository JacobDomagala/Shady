#version 460

layout(constant_id = 0) const int NUM_TEXTURES = 256;

layout(binding = 2) uniform sampler samp;
layout(binding = 3) uniform texture2D textures[NUM_TEXTURES];

layout(location = 0) in VS_OUT
{
   vec3 fPosition;
   vec2 fTexCoord;
   vec2 fMaterialTexCoord;
   vec2 fNormalTexCoord;
   vec4 fColor;
   vec3 fNorm;
   vec4 fTangent;

   flat int fBaseColorSampl;
   flat int fMaterialSampl;
   flat int fNormSampl;
   flat vec4 fBaseColorFactor;
   flat vec3 fMaterialFactors;
}
fs_in;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

void
main()
{
   outPosition = vec4(fs_in.fPosition, 1.0);

   vec3 N = normalize(fs_in.fNorm);
   vec3 tangent = fs_in.fTangent.xyz - N * dot(fs_in.fTangent.xyz, N);
   if (fs_in.fNormSampl >= 0)
   {
      vec3 T;
      vec3 B;
      if (length(tangent) >= 0.0001)
      {
         T = normalize(tangent);
         B = normalize(cross(N, T)) * fs_in.fTangent.w;
      }
      else
      {
         // Derive a frame for assets without tangents and transformed normal UVs.
         vec3 dx = dFdx(fs_in.fPosition);
         vec3 dy = dFdy(fs_in.fPosition);
         vec2 du = dFdx(fs_in.fNormalTexCoord);
         vec2 dv = dFdy(fs_in.fNormalTexCoord);
         vec3 dyPerp = cross(dy, N);
         vec3 dxPerp = cross(N, dx);
         T = dyPerp * du.x + dxPerp * dv.x;
         B = dyPerp * du.y + dxPerp * dv.y;
         float norm = max(dot(T, T), dot(B, B));
         float invNorm = norm > 1e-20 ? inversesqrt(norm) : 0.0;
         T *= invNorm;
         B *= invNorm;
      }
      if (dot(T, T) > 0.0 && dot(B, B) > 0.0)
      {
         vec3 tangentNormal =
            texture(sampler2D(textures[fs_in.fNormSampl], samp), fs_in.fNormalTexCoord).xyz * 2.0 - 1.0;
         tangentNormal.xy *= fs_in.fMaterialFactors.z;
         N = normalize(mat3(T, B, N) * tangentNormal);
      }
   }

   vec4 baseColor = fs_in.fBaseColorFactor * fs_in.fColor;
   if (fs_in.fBaseColorSampl >= 0)
   {
      baseColor *=
         texture(sampler2D(textures[fs_in.fBaseColorSampl], samp), fs_in.fTexCoord);
   }

   float metallic = fs_in.fMaterialFactors.x;
   float roughness = fs_in.fMaterialFactors.y;
   if (fs_in.fMaterialSampl >= 0)
   {
      vec4 materialSample =
         texture(sampler2D(textures[fs_in.fMaterialSampl], samp), fs_in.fMaterialTexCoord);
      roughness *= materialSample.g;
      metallic *= materialSample.b;
   }

   outNormal = vec4(N, clamp(metallic, 0.0, 1.0));
   outAlbedo = vec4(baseColor.rgb, clamp(roughness, 0.04, 1.0));
}
