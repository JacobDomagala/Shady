#include "file_manager.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <fstream>
#include <stb_image.h>
#include <string>

namespace shady::utils {

auto static CreatePath(std::filesystem::path rootPath, std::string_view assetPath)
{
   auto new_path = rootPath / "";
   new_path += std::filesystem::path(assetPath);
   return new_path.string();
}

std::string
FileManager::ReadTextFile(const std::filesystem::path& path)
{
   return ReadTextFile(std::string_view{path.string()});
}

std::string
FileManager::ReadTextFile(std::string_view fileName)
{
   std::ifstream fileHandle(fileName);

   utils::Assert(fileHandle.is_open(),
                 fmt::format("FileManager::ReadTextFile -> {} can't be opened!", fileName));

   std::string returnVal((std::istreambuf_iterator< char >(fileHandle)),
                         std::istreambuf_iterator< char >());
   fileHandle.close();

   utils::Assert(!returnVal.empty(),
                 fmt::format("FileManager::ReadTextFile -> {} is empty!", fileName));

   return returnVal;
}

std::vector< char >
FileManager::ReadBinaryFile(const std::filesystem::path& path)
{
   return ReadBinaryFile(std::string_view{path.string()});
}

std::vector< char >
FileManager::ReadBinaryFile(std::string_view fileName)
{
   std::ifstream fileHandle(fileName, std::ios::binary);

   utils::Assert(fileHandle.is_open(),
                 fmt::format("FileManager::ReadBinaryFile -> {} can't be opened!", fileName));

   const auto size = std::filesystem::file_size(fileName);

   utils::Assert(size, fmt::format("FileManager::ReadBinaryFile -> {} is empty!", fileName));

   std::vector< char > buffer(size);

   fileHandle.read(buffer.data(), static_cast< size_t >(size));

   return buffer;
}

void
FileManager::WriteToFile(std::string_view fileName, std::string_view content)
{
   std::ofstream fileHandle;
   fileHandle.open(fileName);
   fileHandle << content;
}

render::ImageData
FileManager::ReadTexture(std::string_view fileName, bool flipVertical)
{
   const auto pathToImage = CreatePath(TEXTURES_DIR, fileName);
   int force_channels = STBI_rgb_alpha;
   int w, h, n;

   stbi_set_flip_vertically_on_load(flipVertical);

   render::ImageHandleType textureData(stbi_load(pathToImage.c_str(), &w, &h, &n, force_channels),
                                       stbi_image_free);

   utils::Assert(textureData != nullptr,
                 fmt::format("FileManager::LoadImage -> {} can't be opened!", pathToImage));


   return {std::move(textureData), {w, h}, n};
}

} // namespace shady::utils
