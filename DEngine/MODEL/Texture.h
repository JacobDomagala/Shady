#ifndef TEXTURE_H
#define TEXTURE_H


#include<vector>
#include <postprocess.h>
#include<SOIL.h>
#include"..\SHADERS\Shader.h"

enum class TextureQuality : char{
	LOW,
	MEDIUM,
	HIGH
};

enum class textureType : char {
	DIFFUSE_MAP,
	SPECULAR_MAP,
	NORMAL_MAP,
	SHADOW_MAP
};

struct Texture{
	textureType type;
	TextureQuality quality;
	unsigned char* data;
	int width, height;
	const char* samplerName;

	void SetParameter(unsigned int uiSampler, int parameter, int value);
	

	GLuint samplerLocation;
	GLuint textureID;
	GLuint samplerID;
	GLuint frameBufferID;

	aiString path;
	
	void LoadTexture(const char* filePath, const char* samplerName, std::string directory = "\0");
	void CreateDepthBuffer(int width, int height);

	void Use(GLuint programID, unsigned short unit);
	void SetTextureQuality(int quality);
	void SetTextureQuality(TextureQuality quality);
	void CleanUp();
};

#endif