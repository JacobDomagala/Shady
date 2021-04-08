#pragma once

#include "render/texture.hpp"

#include <filesystem>
#include <string_view>


namespace shady::utils {

class FileManager
{
 public:
   static inline const std::filesystem::path ROOT_DIR =
      std::filesystem::path(std::string(CMAKE_ROOT_DIR));
   static inline const std::filesystem::path ASSETS_DIR = ROOT_DIR / "assets";
   static inline const std::filesystem::path TEXTURES_DIR = ASSETS_DIR / "textures";
   static inline const std::filesystem::path SHADERS_DIR = ASSETS_DIR / "shaders";
   static inline const std::filesystem::path MODELS_DIR = ASSETS_DIR / "models";

 public:
   static std::string
   ReadTextFile(const std::filesystem::path& path);

   static std::string
   ReadTextFile(std::string_view fileName);

   static std::vector<char>
   ReadBinaryFile(const std::filesystem::path& path);

   static std::vector<char>
   ReadBinaryFile(std::string_view fileName);

   static void
   WriteToFile(std::string_view fileName, std::string_view content);

   static render::Texture::ImageData
   ReadTexture(std::string_view fileName, bool flipVertical = false);
};

} // namespace shady::utils
