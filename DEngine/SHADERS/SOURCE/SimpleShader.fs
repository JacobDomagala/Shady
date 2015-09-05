#version 430 core

in VS_OUT{
	vec3 fPosition;
	vec3 fNormal;
	vec2 fTexCoord;
}fs_in;

out vec4 outputColor;


uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D normal_map;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;

void main()
{    
	              //AMBIENT
    vec3 ambientLight =  vec3(0.03, 0.03, 0.03);

                //DIFFUSE
    vec3 lightVector = lightPosition - fs_in.fPosition;
    
    vec3 normalizedLight = normalize(lightVector);
    vec3 normalizedNormal = normalize(fs_in.fNormal);

    float dotDiffuse = dot(normalizedLight, normalizedNormal);
    float clampedDiffuse = max(dotDiffuse, 0.0);
    
    vec3 diffuseLight;
    diffuseLight.x = clampedDiffuse;
    diffuseLight.y = clampedDiffuse;
    diffuseLight.z = clampedDiffuse;
	
	
	diffuseLight *= vec3(texture2D(diffuse_map, fs_in.fTexCoord));
    

                //SPECULAR
    
    vec3 reflectedLight = reflect(-normalizedLight,normalizedNormal);
  
    vec3 cameraVector = cameraPosition - fs_in.fPosition;
    vec3 normalizedCamera = normalize(cameraVector);
  
    float dotSpecular = dot(normalizedCamera, reflectedLight);
    float clampedSpecular = max(dotSpecular, 0.0);

    float brightness = pow(clampedSpecular, 16);
    
    
    vec3 specularLight;
    specularLight.x = brightness;
    specularLight.y = brightness;
    specularLight.z = brightness;
	
   
    specularLight *= vec3(texture2D(specular_map, fs_in.fTexCoord));
	vec3 phongLight = specularLight + diffuseLight + ambientLight;
//KONIEC OBLICZEN SWIATLA
   
   

	outputColor = vec4(phongLight, 1.0);// + specularLight;

  
}