#include "Texture.h"


void
Texture::LoadTexture(const char *filePath, const char *samplerName, std::string directory)
{
  this->samplerName = samplerName;
  if (!directory.empty()) {
    std::string filename = std::string(filePath);
    filename = directory + '/' + filename;
    // data = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_AUTO);
  } else {
    // data = SOIL_load_image(filePath, &width, &height, 0, SOIL_LOAD_RGBA);
  }
  if (data == NULL) {
    std::cerr << "Error loading texture from file  " << filePath << std::endl;
    system("pause");
  }

  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);


  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);


  SetTextureQuality(TextureQuality::HIGH);
  quality = TextureQuality::HIGH;
  // SOIL_free_image_data(data);
}

void
Texture::SetParameter(unsigned int uiSampler, int parameter, int value)
{
  glSamplerParameteri(uiSampler, parameter, value);
}

void
Texture::CreateDepthBuffer(int width, int height)
{
  this->width = width;
  this->height = height;
  samplerName = "depth_map";

  glGenFramebuffers(1, &frameBufferID);

  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  GLfloat borderColor[] = { 1.0f, 0.0f, 0.0f, 0.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);


  glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
Texture::Use(GLuint programID, unsigned short unit)
{
  samplerLocation = glGetUniformLocation(programID, samplerName);
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glUniform1i(samplerLocation, unit);
}


void
Texture::SetTextureQuality(TextureQuality quality)
{
  if (quality == TextureQuality::LOW) {
    glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    this->quality = TextureQuality::LOW;
  } else if (quality == TextureQuality::MEDIUM) {
    glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    this->quality = TextureQuality::MEDIUM;
  } else if (quality == TextureQuality::HIGH) {
    GLfloat fLargest;
    // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    // glSamplerParameteri(samplerID, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
    glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    this->quality = TextureQuality::HIGH;
  }
  SetParameter(samplerID, GL_TEXTURE_WRAP_S, GL_REPEAT);
  SetParameter(samplerID, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void
Texture::CleanUp()
{
  glDeleteSamplers(1, &samplerID);
  glDeleteTextures(1, &textureID);
}
