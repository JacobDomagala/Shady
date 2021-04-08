#include "file_manager.hpp"
#include "trace/logger.hpp"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <fstream>
#include <string>
#include <stb_image.h>

namespace shady::utils {

std::string
FileManager::ReadTextFile(const std::filesystem::path& path)
{
   return ReadTextFile(std::string_view{path.string()});
}

std::string
FileManager::ReadTextFile(std::string_view fileName)
{
   std::ifstream fileHandle(fileName);

   if (!fileHandle.is_open())
   {
      trace::Logger::Fatal("FileManager::ReadTextFile -> {} can't be opened!", fileName);
   }

   std::string returnVal((std::istreambuf_iterator< char >(fileHandle)),
                         std::istreambuf_iterator< char >());
   fileHandle.close();

   if (returnVal.empty())
   {
      trace::Logger::Fatal("FileManager::ReadTextFile -> {} is empty!", fileName);
   }

   return returnVal;
}

std::vector<char>
FileManager::ReadBinaryFile(const std::filesystem::path& path)
{
   return ReadBinaryFile(std::string_view{path.string()});
}

std::vector<char>
FileManager::ReadBinaryFile(std::string_view fileName)
{
   std::ifstream fileHandle(fileName, std::ios::binary);

   if (!fileHandle.is_open())
   {
      trace::Logger::Fatal("FileManager::ReadBinaryFile -> {} can't be opened!", fileName);
      return {};
   }

   const auto size = std::filesystem::file_size(fileName);

   if (!size)
   {
      trace::Logger::Fatal("FileManager::ReadBinaryFile -> {} is empty!", fileName);
      return {};
   }


   std::vector< char > buffer(size);

   fileHandle.read(buffer.data(), static_cast<size_t>(size));

   return buffer;
}

void
FileManager::WriteToFile(std::string_view fileName, std::string_view content)
{
   std::ofstream fileHandle;
   fileHandle.open(fileName);
   fileHandle << content;
}

render::Texture::ImageData
FileManager::ReadTexture(std::string_view fileName, bool flipVertical)
{
   const auto pathToImage = std::filesystem::path(TEXTURES_DIR / fileName).string();
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
