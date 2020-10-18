#pragma once

#include "render/texture.hpp"

#include <filesystem>
#include <string>

namespace shady::utils {

class FileManager
{
 public:
   static inline const std::filesystem::path ROOT_DIR =
      std::filesystem::current_path().parent_path().parent_path();
   static inline const std::filesystem::path ASSETS_DIR = ROOT_DIR / "assets";
   static inline const std::filesystem::path TEXTURES_DIR = ASSETS_DIR / "textures";
   static inline const std::filesystem::path SHADERS_DIR = ASSETS_DIR / "shaders";

   enum class FileType
   {
      BINARY = 0,
      TEXT
   };

 public:
   static std::string
   ReadFile(const std::filesystem::path& path, FileType type = FileType::TEXT);

   static std::string
   ReadFile(const std::string& fileName, FileType type = FileType::TEXT);

   static void
   WriteToFile(const std::string& fileName, const std::string& content,
               FileType type = FileType::TEXT);

   static render::Texture::ImageData
   ReadTexture(const std::string& fileName, bool flipVertical = false);
};

} // namespace shady::utils
