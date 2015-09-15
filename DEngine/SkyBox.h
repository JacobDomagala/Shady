#ifndef SKYBOX_H
#define SKYBOX_H

#include"DISPLAY\Camera.h"
#include"SHADERS\Shader.h"
#include<vector>
#include<SOIL.h>
#include <gtc/type_ptr.hpp>

class SkyBox{
private:
	std::vector<const GLchar*> faces;
	GLuint textureID;
	Shader skyBoxShaders;
	GLuint skyboxVAO, skyboxVBO;
public:
	void LoadCubeMap(const char* filePath);
	void Draw(Display* window, Camera camera);
};

#endif