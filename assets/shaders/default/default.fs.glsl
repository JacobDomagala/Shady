#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader_int64 : enable

in VS_OUT
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
fs_in;

out vec4 color;

void
main(void)
{
   sampler2D diff = sampler2D(fs_in.fDiffSampl);
   sampler2D norm = sampler2D(fs_in.fNormSampl);
   sampler2D spec = sampler2D(fs_in.fSpecSampl);

   vec3 fNormal = texture(norm, fs_in.fTexCoord).rgb;
   fNormal = normalize(fs_in.TBN * (fNormal * (255.0f / 128.0f) - 1.0f));

   // AMBIENT
   vec3 ambientLight = vec3(0.03f, 0.03f, 0.03f);

   // DIFFUSE
   vec3 lightVector = fs_in.fLightPosition - fs_in.fPosition;

   vec3 normalizedLight = normalize(lightVector);
   vec3 normalizedNormal = normalize(fNormal);

   float dotDiffuse = dot(normalizedLight, normalizedNormal);
   float clampedDiffuse = max(dotDiffuse, 0.0f);

   vec3 diffuseLight = vec3(clampedDiffuse);

   diffuseLight *= vec3(texture(diff, fs_in.fTexCoord));

   // SPECULAR
   vec3 cameraVector = fs_in.fCameraPosition - fs_in.fPosition;
   vec3 normalizedCamera = normalize(cameraVector);
   vec3 reflectedLight = normalize(normalizedLight + normalizedCamera);

   float dotSpecular = dot(normalizedNormal, reflectedLight);
   float clampedSpecular = max(dotSpecular, 0.0);

   float brightness = pow(clampedSpecular, 32);

   vec3 specularLight = vec3(brightness);

   specularLight *= vec3(texture(spec, fs_in.fTexCoord));

   //OUTPUT
   vec3 lighting = (ambientLight + (diffuseLight + specularLight));

   color = vec4(lighting, 1.0f);
}
