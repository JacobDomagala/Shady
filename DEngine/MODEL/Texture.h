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
	static unsigned short textureCount;
	

	unsigned char* data;
	int width, height;


	unsigned int unit;
	GLuint samplerLocation;
	GLuint textureID;
	GLuint samplerID;

	void SetParameter(unsigned int uiSampler, int parameter, int value);
	


public:

	GLuint id;
	std::string type;
	aiString path;
	
	 GLuint LoadTexture(const char* filePath, std::string directory, GLuint program);
	void CleanUp(); 

	
	void Use();

	unsigned int GetUnit() { return unit; }
	void SetTextureQuality(int quality);
};

#endif