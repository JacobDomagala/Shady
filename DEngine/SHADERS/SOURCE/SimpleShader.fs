#version 430 core


uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform sampler2D sampler;
uniform float time;

struct material{
sampler2D diffuseMap;
sampler2D specularMap;
float shinePower;
};
uniform material meshMaterial;

in vec3 fPosition;
in vec3 fNormal;
in vec2 fUv;

out vec4 outputColor;

void main()
{
//POCZATEK OBLICZEN SWIATLA

                //AMBIENT
    vec3 ambientLight = vec3(0.3, 0.3, 0.3);

                //DIFFUSE
    vec3 lightVector = lightPosition - fPosition;
    
    vec3 normalizedLight = normalize(lightVector);
    vec3 normalizedNormal = normalize(fNormal);

    float dotDiffuse = dot(normalizedLight, normalizedNormal);
    float clampedDiffuse = max(dotDiffuse, 0.0);
    
    vec3 diffuseLight;
    diffuseLight.x = clampedDiffuse;
    diffuseLight.y = clampedDiffuse;
    diffuseLight.z = clampedDiffuse;
    

                //SPECULAR
    
    vec3 reflectedLight = reflect(-normalizedLight,normalizedNormal);
  
    vec3 cameraVector = cameraPosition - fPosition;
    vec3 normalizedCamera = normalize(cameraVector);
  
    float dotSpecular = dot(reflectedLight, normalizedCamera);
    float clampedSpecular = max(dotSpecular, 0.0);

    float braightness = pow(clampedSpecular, meshMaterial.shinePower);
    
    
    vec3 specularLight;
    specularLight.x = braightness;
    specularLight.y = braightness;
    specularLight.z = braightness;
   

   
    
                //PHONG'S LIGHTINING

    vec3 finalLight =ambientLight + diffuseLight + specularLight;
//KONIEC OBLICZEN SWIATLA
   
   

	outputColor = texture2D(sampler, fUv) * vec4(finalLight, 1.0) ;
}