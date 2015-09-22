#include "Texture.h"


void Texture::LoadTexture(const char* filePath, char* name, std::string directory)
{
	samplerName = name;
	if (!directory.empty())
	{
		std::string filename = std::string(filePath);
		filename = directory + '/' + filename;
		data = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
	}
	else
	{
		data = SOIL_load_image(filePath, &width, &height, 0, SOIL_LOAD_RGBA);
	}
	if (data == NULL)
	{
		std::cerr << "Error loading texture from file  " << filePath<< std::endl;
		system("pause");
	}
	
 	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	

	SetTextureQuality(HIGH);
	quality = HIGH;
	SOIL_free_image_data(data);

}

void Texture::SetParameter(unsigned int uiSampler, int parameter, int value)
{
	glSamplerParameteri(uiSampler, parameter, value);
}

void Texture::CreateTexture(int width, int height)
{
	glGenFramebuffers(1, &frameBufferID);

	glGenBuffers(1, &textureID);
	glBindBuffer(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Texture::Use(GLuint programID, unsigned short unit)
{
	samplerLocation = glGetUniformLocation(programID, samplerName);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUniform1i(samplerLocation, unit);
}


void Texture::SetTextureQuality(int quality)
{
	if (quality == LOW)
	{
		glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		this->quality = LOW;
	}
	else if (quality == MEDIUM)
	{
		glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		this->quality = MEDIUM;
	}
	else if (quality == HIGH)
	{
		GLfloat fLargest;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
		glSamplerParameteri(samplerID, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
		glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		this->quality = HIGH;
	}
	SetParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	SetParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Texture::CleanUp()
{
	glDeleteSamplers(1, &samplerID);
	glDeleteTextures(1, &textureID);
}
