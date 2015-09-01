#include "Texture.h"

unsigned short Texture::textureCount;
void Texture::LoadTexture(char* filePath, Shader* program)
{
	 data = SOIL_load_image(filePath, &width, &height, 0, SOIL_LOAD_RGBA);

	if (data == NULL){
			std::cerr << "Blad wczytywania tekstury z pliku " << filePath<< std::endl;
			system("pause");
		}
	
	
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	glGenSamplers(1, &samplerID);

	SOIL_free_image_data(data);
	
	samplerLocation = glGetUniformLocation(program->GetProgramID(), "sampler");

	unit = textureCount;
	textureCount++;

}
void Texture::SetParameter(unsigned int uiSampler, int parameter, int value){
	glSamplerParameteri(uiSampler, parameter, value);
}


void Texture::Use(){
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glBindSampler(unit, samplerID);
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