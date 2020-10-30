#version 450 core

uniform sampler2D u_textures[32];

in VS_OUT
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
fs_in;

out vec4 color;

// Material properties
uniform vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7);
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 128.0;
uniform vec3 ambient = vec3(0.1, 0.1, 0.1);

void
main(void)
{
   vec3 fNormal = texture(u_textures[fs_in.fNormTex], fs_in.fTexCoord).rgb;
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

   diffuseLight *= vec3(texture2D(u_textures[fs_in.fDiffTex], fs_in.fTexCoord));

   // SPECULAR
   vec3 cameraVector = fs_in.fCameraPosition - fs_in.fPosition;
   vec3 normalizedCamera = normalize(cameraVector);
   vec3 reflectedLight = normalize(normalizedLight + normalizedCamera);

   float dotSpecular = dot(normalizedNormal, reflectedLight);
   float clampedSpecular = max(dotSpecular, 0.0);

   float brightness = pow(clampedSpecular, 32);

   vec3 specularLight = vec3(brightness);

   specularLight *= vec3(texture2D(u_textures[fs_in.fSpecTex], fs_in.fTexCoord));

   //OUTPUT
   vec3 lighting = (ambientLight + (diffuseLight + specularLight));

   color = vec4(lighting, 1.0f);
}
