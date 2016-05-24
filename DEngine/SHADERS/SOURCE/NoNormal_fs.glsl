#version 420 core

in VS_OUT{
	vec3 fPosition;
	vec3 fNormal;
	vec2 fTexCoord;
	vec3 fLightPosition;
	vec3 fCameraPosition;
	mat3 TBN;
}fs_in;

out vec4 outputColor;


uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D normal_map;

void main()
{   
	vec4 diffuseMap = texture2D(diffuse_map, fs_in.fTexCoord);
	vec4 ambient = diffuseMap * vec4(0.08f, 0.08f, 0.00f, 1.0f);
	

	vec3 lightDir = normalize(fs_in.fLightPosition - fs_in.fPosition);

	vec3 normal = normalize(fs_in.TBN *(texture2D(normal_map, fs_in.fTexCoord).xyz * 2.0f - 1.0f));
	float diff = max(dot(lightDir, normal), 0.0f);
	vec4 diffuse = diffuseMap * vec4(vec3(diff), 1.0f);

	vec3 eyeDir = normalize(fs_in.fCameraPosition - fs_in.fPosition);
	vec3 reflected = normalize(reflect(-lightDir, normal));
	float spec = pow(max(dot(reflected, eyeDir), 0.0f), 64);
	vec4 specular = texture2D(specular_map, fs_in.fTexCoord) * vec4(vec3(spec), 1.0f);

	vec4 light = ambient + diffuse + specular;
	outputColor = light;
}