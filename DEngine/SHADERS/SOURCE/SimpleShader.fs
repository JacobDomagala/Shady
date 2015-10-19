#version 430 core

in VS_OUT{
	vec3 fPosition;
	vec2 fTexCoord;
	vec3 fCameraPosition;
	vec3 fLightPosition;
	mat3 TBN;
	vec4 fLightSpacePosition;
}fs_in;

out vec4 fColor;

uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D normal_map;
uniform sampler2DShadow depth_map;

uniform float time;

float ShadowCalculation(vec4 fragment, vec3 normal)
{
	float shadow = 0.0f;

	for(int i = -1; i < 2; i++)
	{
		for(int j = -1; j < 2; j++)
		{
			shadow += textureProjOffset(depth_map, fragment, ivec2(i,j));
		}
	}
	shadow /= 9.0f;
    return shadow;
}

void main()
{    
	vec3 fNormal = texture(normal_map, fs_in.fTexCoord).rgb;
	fNormal = normalize(fs_in.TBN * (fNormal * (255.0f/128.0f) - 1.0f));  
	
//AMBIENT
    vec3 ambientLight =  vec3(0.03f, 0.03f, 0.03f);

//DIFFUSE
    vec3 lightVector = fs_in.fLightPosition - fs_in.fPosition;
    
    vec3 normalizedLight = normalize(lightVector);
    vec3 normalizedNormal = normalize(fNormal);

    float dotDiffuse = dot(normalizedLight, normalizedNormal);
    float clampedDiffuse = max(dotDiffuse, 0.0f);
    
    vec3 diffuseLight = vec3(clampedDiffuse);
	
	diffuseLight *= vec3(texture2D(diffuse_map, fs_in.fTexCoord));
   
//SPECULAR				
    vec3 cameraVector = fs_in.fCameraPosition - fs_in.fPosition;
    vec3 normalizedCamera = normalize(cameraVector);
	vec3 reflectedLight = normalize(normalizedLight + normalizedCamera);
  
    float dotSpecular = dot(normalizedNormal, reflectedLight);
    float clampedSpecular = max(dotSpecular, 0.0);

    float brightness = pow(clampedSpecular, 32);
       
    vec3 specularLight = vec3(brightness);
    
    specularLight *= vec3(texture2D(specular_map, fs_in.fTexCoord));
	
//SHADOWS 
	float shadow = ShadowCalculation(fs_in.fLightSpacePosition, fNormal);       
    vec3 lighting = (ambientLight + shadow * (diffuseLight + specularLight));
	 
	fColor = vec4(lighting, 1.0f);
	
}