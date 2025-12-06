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

float textureProj(vec4 P, float layer, vec2 offset)
{
	float shadow = 1.0;
	vec4 shadowCoord = P / P.w;
	shadowCoord.st = shadowCoord.st * 0.5 + 0.5;

	if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
	{
		float dist = texture(samplerShadowMap, shadowCoord.st + offset).r;
		if (shadowCoord.w > 0.0 && dist < shadowCoord.z)
		{
			shadow = ubo.debug.shadowFactor;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc, float layer)
{
	ivec2 texDim = textureSize(samplerShadowMap, 0).xy;
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;

	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, layer, vec2(dx*x, dy*y));
			count++;
		}

	}
	return shadowFactor / count;
}

vec3 shadow(vec3 fragcolor, vec3 fragpos) {
	for(int i = 0; i < LIGHT_COUNT; ++i)
	{
		vec4 shadowClip	= ubo.lights[i].viewMatrix * vec4(fragpos, 1.0);

		float shadowFactor;
		if(ubo.debug.pcfShadow > 0)
			shadowFactor = filterPCF(shadowClip, i);
		else
			shadowFactor = textureProj(shadowClip, i, vec2(0.0));

		fragcolor *= shadowFactor;
	}
	return fragcolor;
}

void
main()
{
   // Get G-Buffer values
   vec4 positionSample = texture(samplerPosition, inUV);
   if (positionSample.a == 0.0)
   {
      discard;
   }

   vec3 fragPos = positionSample.rgb;
   vec3 normal = texture(samplerNormal, inUV).rgb;
   vec4 albedo = texture(samplerAlbedo, inUV);
   vec4 shadow_sample = texture(samplerShadowMap, inUV);

   // Debug display
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
            outFragcolor.rgb = albedo.aaa;
            break;
         case 5:
            outFragcolor.rgb = vec3(shadow_sample.r);
            break;
      }
      outFragcolor.a = 1.0;
      return;
   }

   // Ambient part
   vec3 fragcolor = albedo.rgb * ubo.debug.ambientLight;

   for (int i = 0; i < LIGHT_COUNT; ++i)
   {
      // Vector to light
      vec3 L = ubo.lights[i].position.xyz - fragPos;
      // Distance from light to fragment position
      float dist = length(L);

      // Viewer to fragment
      vec3 V = ubo.viewPos.xyz - fragPos;
      V = normalize(V);

      // if(dist < ubo.lights[i].radius)
      {
         // Light to fragment
         L = normalize(L);

         // Attenuation
         float atten = 1.0f; //ubo.lights[i].radius / (pow(dist, 2.0) + 1.0);

         // Diffuse part
         vec3 N = normalize(normal);
         float NdotL = max(0.0, dot(N, L));
         vec3 diff = ubo.lights[i].color.rgb * albedo.rgb * NdotL * atten;

         // Specular part
         // Specular map values are stored in alpha of albedo mrt
         vec3 R = reflect(-L, N);
         float NdotR = max(0.0, dot(R, V));
         vec3 spec = ubo.lights[i].color.rgb * albedo.a * pow(NdotR, 16.0) * atten;

         fragcolor += diff + spec;
      }
   }

   // Shadow calculations in a separate pass
	fragcolor = shadow(fragcolor, fragPos);

   outFragcolor = vec4(fragcolor, 1.0);
}
