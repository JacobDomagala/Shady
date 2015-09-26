#ifndef TEXTURE_H
#define TEXTURE_H


#include<vector>
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
	
	TextureQuality quality;
	unsigned char* data;
	int width, height;
	char* samplerName;

	void SetParameter(unsigned int uiSampler, int parameter, int value);
	
public:
	
	GLuint samplerLocation;
	GLuint textureID;
	GLuint samplerID;
	GLuint frameBufferID;

	std::string type;
	aiString path;
	
	void LoadTexture(const char* filePath, char* textureType, std::string directory = "\0");
	void CreateDepthBuffer(int width, int height);

	void Use(GLuint programID, unsigned short unit);
	void SetTextureQuality(int quality);
	void CleanUp();
};

#endif