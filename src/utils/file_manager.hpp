#pragma once

#include "render/texture.hpp"

#include <filesystem>
#include <string>


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
   ReadTextFile(const std::string& fileName);

   static std::vector<char>
   ReadBinaryFile(const std::filesystem::path& path);

   static std::vector<char>
   ReadBinaryFile(const std::string& fileName);

   static void
   WriteToFile(const std::string& fileName, const std::string& content);

   static render::Texture::ImageData
   ReadTexture(const std::string& fileName, bool flipVertical = false);
};

} // namespace shady::utils
