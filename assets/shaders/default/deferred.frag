#version 460

layout(binding = 4) uniform sampler2D samplerAlbedo;
layout(binding = 5) uniform sampler2D samplerPosition;
layout(binding = 6) uniform sampler2D samplerNormal;
layout(binding = 8) uniform sampler2D samplerShadowMap;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outFragcolor;

#define LIGHT_COUNT 1

struct Light
{
   vec4 position;
   vec4 target;
   vec4 color;
   mat4 viewMatrix;
};

struct DebugData
{
   int displayDebugTarget;
   int pcfShadow;
   float ambientLight;
   float shadowFactor;
};

layout(binding = 7) uniform UBO
{
   Light lights[LIGHT_COUNT];
   vec4 viewPos;
   DebugData debug;
}
ubo;

float
textureProj(vec4 P, vec2 offset)
{
   float shadow = 1.0;
   vec4 shadowCoord = P / P.w;
   shadowCoord.st = shadowCoord.st * 0.5 + 0.5;

   if (shadowCoord.z >= 0.0 && shadowCoord.z <= 1.0)
   {
      float dist = texture(samplerShadowMap, shadowCoord.st + offset).r;
      if (shadowCoord.w > 0.0 && dist < shadowCoord.z)
      {
         shadow = ubo.debug.shadowFactor;
      }
   }
   return shadow;
}

float
filterPCF(vec4 shadowClip)
{
   ivec2 texDim = textureSize(samplerShadowMap, 0).xy;
   vec2 texelSize = vec2(1.5) / vec2(texDim);
   float shadowFactor = 0.0;
   int count = 0;

   for (int x = -1; x <= 1; ++x)
   {
      for (int y = -1; y <= 1; ++y)
      {
         shadowFactor += textureProj(shadowClip, vec2(x, y) * texelSize);
         ++count;
      }
   }
   return shadowFactor / count;
}

float
shadowFactor(vec3 fragPos)
{
   vec4 shadowClip = ubo.lights[0].viewMatrix * vec4(fragPos, 1.0);
   return ubo.debug.pcfShadow > 0 ? filterPCF(shadowClip)
                                 : textureProj(shadowClip, vec2(0.0));
}

const float PI = 3.14159265359;

float
distributionGGX(vec3 N, vec3 H, float roughness)
{
   float a = roughness * roughness;
   float a2 = a * a;
   float NdotH = max(dot(N, H), 0.0);
   float denominator = NdotH * NdotH * (a2 - 1.0) + 1.0;
   return a2 / max(PI * denominator * denominator, 0.0001);
}

float
geometrySchlickGGX(float NdotV, float roughness)
{
   float r = roughness + 1.0;
   float k = (r * r) / 8.0;
   return NdotV / max(NdotV * (1.0 - k) + k, 0.0001);
}

float
geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
   return geometrySchlickGGX(max(dot(N, V), 0.0), roughness)
          * geometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

vec3
fresnelSchlick(float cosTheta, vec3 F0)
{
   return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void
main()
{
   vec4 positionSample = texture(samplerPosition, inUV);
   if (positionSample.a == 0.0)
   {
      discard;
   }

   vec3 fragPos = positionSample.rgb;
   vec4 normalSample = texture(samplerNormal, inUV);
   vec3 normal = normalSample.rgb;
   float metallic = normalSample.a;
   vec4 albedo = texture(samplerAlbedo, inUV);
   float roughness = albedo.a;

   if (ubo.debug.displayDebugTarget > 0)
   {
      switch (ubo.debug.displayDebugTarget)
      {
         case 1:
            outFragcolor.rgb = fragPos;
            break;
         case 2:
            outFragcolor.rgb = normal;
            break;
         case 3:
            outFragcolor.rgb = albedo.rgb;
            break;
         case 4:
            outFragcolor.rgb = vec3(roughness);
            break;
         case 5:
            outFragcolor.rgb = vec3(texture(samplerShadowMap, inUV).r);
            break;
      }
      outFragcolor.a = 1.0;
      return;
   }

   vec3 N = normalize(normal);
   vec3 V = normalize(ubo.viewPos.xyz - fragPos);
   vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);
   vec3 fragcolor = albedo.rgb * ubo.debug.ambientLight * (1.0 - metallic);

   for (int i = 0; i < LIGHT_COUNT; ++i)
   {
      vec3 lightDelta = ubo.lights[i].position.xyz - fragPos;
      if (length(lightDelta) <= 0.0001)
      {
         continue;
      }

      vec3 L = normalize(lightDelta);
      vec3 H = normalize(V + L);
      float NdotL = max(dot(N, L), 0.0);
      float NdotV = max(dot(N, V), 0.0);
      vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
      float D = distributionGGX(N, H, roughness);
      float G = geometrySmith(N, V, L, roughness);
      vec3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, 0.0001);

      vec3 diffuseWeight = (vec3(1.0) - F) * (1.0 - metallic);
      vec3 diffuse = diffuseWeight * albedo.rgb / PI;
      fragcolor += (diffuse + specular) * ubo.lights[i].color.rgb * NdotL;
   }

   fragcolor *= shadowFactor(fragPos);
   outFragcolor = vec4(fragcolor, 1.0);
}
