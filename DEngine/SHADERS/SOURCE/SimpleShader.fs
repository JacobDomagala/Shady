#version 330 core

in VS_OUT{
	vec3 position;
	vec3 normal;
	vec2 texCoord;
}fs_in;

out vec4 color;

const int diff = 2;
const int spec = 2;
uniform sampler2D [diff]diffuse_map;
uniform sampler2D [spec]specular_map;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

void main()
{    
	               //AMBIENT
    vec3 ambientLight = vec3(0.3, 0.3, 0.3);

                //DIFFUSE
    vec3 lightVector = lightPosition - fs_in.position;
    
    vec3 normalizedLight = normalize(lightVector);
    vec3 normalizedNormal = normalize(fs_in.normal);

    float dotDiffuse = dot(normalizedLight, normalizedNormal);
    float clampedDiffuse = max(dotDiffuse, 0.0);
    
    vec3 diffuseLight;
    diffuseLight.x = clampedDiffuse;
    diffuseLight.y = clampedDiffuse;
    diffuseLight.z = clampedDiffuse;
    

                //SPECULAR
    
    vec3 reflectedLight = reflect(-normalizedLight,normalizedNormal);
  
    vec3 cameraVector = cameraPosition - fs_in.position;
    vec3 normalizedCamera = normalize(cameraVector);
  
    float dotSpecular = dot(reflectedLight, normalizedCamera);
    float clampedSpecular = max(dotSpecular, 0.0);

    float brightness = pow(clampedSpecular, 16);
    
    
    vec3 specularLight;
    specularLight.x = brightness;
    specularLight.y = brightness;
    specularLight.z = brightness;
   

   
    
                //PHONG'S LIGHTINING

    vec3 finalLight = ambientLight + diffuseLight + specularLight;
//KONIEC OBLICZEN SWIATLA
   
   

	color = texture2D(diffuse_map[0], fs_in.texCoord) * vec4(finalLight, 1.0) ;

  
}