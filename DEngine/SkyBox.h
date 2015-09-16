#ifndef SKYBOX_H
#define SKYBOX_H

#include"DISPLAY\Camera.h"
#include"SHADERS\Shader.h"
#include<vector>
#include<SOIL.h>
#include <gtc/type_ptr.hpp>

class SkyBox{
public:
	std::vector<std::string> faces;
	GLuint textureID;
	Shader skyBoxShaders;
	GLuint skyboxVAO, skyboxVBO;

	void LoadCubeMap(std::string folderPath);
	void Draw(Display* window, Camera camera, Shader shader);
};

#endif