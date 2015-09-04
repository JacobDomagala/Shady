#ifndef TEXTURE_H
#define TEXTURE_H

#include <postprocess.h>
#include<SOIL.h>
#include"..\SHADERS\Shader.h"

enum TextureQuality{
	LOW,
	MEDIUM,
	HIGH
};

class Texture
{
private:
	
	unsigned char* data;
	int width, height;


	void SetParameter(unsigned int uiSampler, int parameter, int value);
	


public:
	
	GLuint samplerLocation;
	GLuint textureID;
	GLuint samplerID;
	
	std::string type;
	aiString path;
	
	void LoadTexture(const char* filePath, std::string directory, GLuint program);
	void Use(unsigned short unit);	
	void SetTextureQuality(int quality);
	void CleanUp();
};

#endif