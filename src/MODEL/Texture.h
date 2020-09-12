#ifndef TEXTURE_H
#define TEXTURE_H


#include "SHADERS/Shader.h"
#include <assimp/postprocess.h>
#include <vector>

enum class TextureQuality : char
{
   LOW,
   MEDIUM,
   HIGH
};

enum class textureType : char
{
   DIFFUSE_MAP,
   SPECULAR_MAP,
   NORMAL_MAP,
   SHADOW_MAP
};

struct Texture
{
   textureType type;
   TextureQuality quality;
   unsigned char* data;
   int width, height;
   const char* samplerName;

   void
   SetParameter(unsigned int uiSampler, int parameter, int value);


   uint32_t samplerLocation;
   uint32_t textureID;
   uint32_t samplerID;
   uint32_t frameBufferID;

   aiString path;

   void
   LoadTexture(const char* filePath, const char* samplerName, std::string directory = "\0");
   void
   CreateDepthBuffer(int width, int height);

   void
   Use(uint32_t programID, unsigned short unit);
   void
   SetTextureQuality(int quality);
   void
   SetTextureQuality(TextureQuality quality);
   void
   CleanUp();
};

#endif