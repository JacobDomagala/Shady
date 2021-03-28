#include "file_manager.hpp"
#include "trace/logger.hpp"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <fstream>
#include <stb_image.h>

namespace shady::utils {

std::string
FileManager::ReadFile(const std::filesystem::path& path, FileType type)
{
   return ReadFile(path.u8string(), type);
}

std::string
FileManager::ReadFile(const std::string& fileName, FileType /*type*/)
{
   std::ifstream fileHandle;
   fileHandle.open(fileName, std::ifstream::in);

   if (!fileHandle.is_open())
   {
      trace::Logger::Fatal("FileManager::ReadFile -> {} can't be opened!", fileName);
   }

   std::string returnVal((std::istreambuf_iterator< char >(fileHandle)),
                         std::istreambuf_iterator< char >());
   fileHandle.close();

   if (returnVal.empty())
   {
      trace::Logger::Fatal("FileManager::ReadFile -> {} is empty!", fileName);
   }

   return returnVal;
}

void
FileManager::WriteToFile(const std::string& fileName, const std::string& content, FileType /*type*/)
{
   std::ofstream fileHandle;
   fileHandle.open(fileName);
   fileHandle << content;
}

render::Texture::ImageData
FileManager::ReadTexture(const std::string& fileName, bool flipVertical)
{
   const auto pathToImage = std::filesystem::path(TEXTURES_DIR / fileName).u8string();
   int force_channels = 0;
   int w, h, n;

   stbi_set_flip_vertically_on_load(flipVertical);

   render::Texture::ImageHandleType textureData(
      stbi_load(pathToImage.c_str(), &w, &h, &n, force_channels), stbi_image_free);

   if (!textureData)
   {
      trace::Logger::Fatal("FileManager::LoadImage -> {} can't be opened!", pathToImage);
   }

   return {std::move(textureData), {w, h}, n};
}

} // namespace shady::utils
