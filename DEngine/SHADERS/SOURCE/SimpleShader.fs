#version 430 core

in VS_OUT{
	vec3 fPosition;
	vec2 fTexCoord;
	vec3 fCameraPosition;
	vec3 fLightPosition;
	mat3 TBN;
	vec4 fLightSpacePosition;
}fs_in;

out vec4 outputColor;

uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D normal_map;
uniform sampler2D depth_map;

float ShadowCalculation(vec4 fragment, vec3 normal)
{
	
	vec3 projCoords = fragment.xyz / fragment.w;
	projCoords = projCoords * 0.5f + 0.5f;

	float closestDepth = texture(depth_map, projCoords.xy).r; 
	
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
	
    // Check whether current frag pos is in shadow
	float bias = max(0.05f * (1.0 - dot(normal, fs_in.fLightPosition)), 0.005f);
    float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(depth_map, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(depth_map, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;

	if(projCoords.z > 1.0)
        shadow = 0.0;
    
    
    return shadow;
}

void main()
{    
	vec3 fNormal = texture(normal_map, fs_in.fTexCoord).rgb;
	fNormal = normalize(fs_in.TBN * (fNormal * (255.0/128.0) - 1.0));  
	
//AMBIENT
    vec3 ambientLight =  vec3(0.03, 0.03, 0.03);

//DIFFUSE
    vec3 lightVector = fs_in.fLightPosition - fs_in.fPosition;
    
    vec3 normalizedLight = normalize(lightVector);
    vec3 normalizedNormal = normalize(fNormal);

    float dotDiffuse = dot(normalizedLight, normalizedNormal);
    float clampedDiffuse = max(dotDiffuse, 0.0);
    
    vec3 diffuseLight;
    diffuseLight.x = clampedDiffuse;
    diffuseLight.y = clampedDiffuse;
    diffuseLight.z = clampedDiffuse;
	
	
	diffuseLight *= vec3(texture2D(diffuse_map, fs_in.fTexCoord));
    

//SPECULAR				
    //vec3 reflectedLight = reflect(-normalizedLight,normalizedNormal);
	
	
    vec3 cameraVector = fs_in.fCameraPosition - fs_in.fPosition;
    vec3 normalizedCamera = normalize(cameraVector);
	vec3 reflectedLight = normalize(normalizedLight + normalizedCamera);
  
    float dotSpecular = dot(normalizedNormal, reflectedLight);
    float clampedSpecular = max(dotSpecular, 0.0);

    float brightness = pow(clampedSpecular, 32);
       
    vec3 specularLight = vec3(brightness);
    
	
    specularLight *= vec3(texture2D(specular_map, fs_in.fTexCoord));
	
 
	float shadow = ShadowCalculation(fs_in.fLightSpacePosition, fNormal);       
    vec3 lighting = (ambientLight + (1.0 - shadow) * (diffuseLight + specularLight));
	 
	outputColor = vec4(lighting, 1.0);
	
}