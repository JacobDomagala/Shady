#include "Texture.h"


void Texture::LoadTexture(const char* filePath, std::string directory, GLuint program, char* name)
{
	samplerName = name;
	std::string filename = std::string(filePath);
	filename = directory + '/' + filename;
	 data = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);

	if (data == NULL){
			std::cerr << "Blad wczytywania tekstury z pliku " << filePath<< std::endl;
			system("pause");
		}
	
 	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	


	SetTextureQuality(HIGH);
	SOIL_free_image_data(data);

}
void Texture::SetParameter(unsigned int uiSampler, int parameter, int value){
	glSamplerParameteri(uiSampler, parameter, value);
}


void Texture::Use(GLuint programID, unsigned short unit){
	samplerLocation = glGetUniformLocation(programID, samplerName);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUniform1i(samplerLocation, unit);
	
}

void Texture::CleanUp(){
	glDeleteSamplers(1, &samplerID);
	glDeleteTextures(1, &textureID);
}
void Texture::SetTextureQuality(int quality){
	if (quality == LOW){
		glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else if (quality == MEDIUM){
		glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else if (quality == HIGH){
		GLfloat fLargest;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
		glSamplerParameteri(samplerID, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
		glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	SetParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	SetParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}