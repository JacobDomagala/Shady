#ifndef LIGHT_H
#define LIGHT_H

#include"DISPLAY\Camera.h"
#include"MODEL\Texture.h"
#include <gtc/type_ptr.hpp>

enum lightType {
	DIRECTIONAL_LIGHT, 
	POINT_LIGHT,
	SPOTLIGHT
};

class Light{
	Texture shadowTexture;
	int shadowTextureWidth;
	int shadowTextureHeight;

	glm::vec3 color;
	glm::vec3 position;
	float intensity;

	glm::mat4 modelMatrix;
	glm::mat4 lightSpaceMatrix;
	glm::mat4 biasMatrix;
	glm::mat4 shadowMatrix;
public:
	Light(glm::vec3 position, glm::vec3 color, lightType type);
	void StartDrawingShadows(GLuint programID);
	void StopDrawingShadows();
	
	inline glm::mat4 GetLightMatrix() { return shadowMatrix; }
	inline GLuint GetShadowMapID() { return shadowTexture.textureID; }
	inline glm::vec3 GetLightPosition() { return position; }

};

#endif