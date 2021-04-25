#pragma once

#include <filesystem>
#include <string_view>
#include <glm/glm.hpp>
#include <memory>
#include <functional>

namespace shady::render {

using ImageHandleType = std::unique_ptr< uint8_t[], std::function< void(uint8_t*) > >;

struct ImageData
{
   ImageHandleType m_bytes;
   glm::uvec2 m_size;
   int32_t m_channels;
};
} // namespace shady::render

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
   static inline const std::filesystem::path FONTS_DIR = ASSETS_DIR / "fonts";

 public:
   static std::string
   ReadTextFile(const std::filesystem::path& path);

   static std::string
   ReadTextFile(std::string_view fileName);

   static std::vector< char >
   ReadBinaryFile(const std::filesystem::path& path);

   static std::vector< char >
   ReadBinaryFile(std::string_view fileName);

   static void
   WriteToFile(std::string_view fileName, std::string_view content);

   static render::ImageData
   ReadTexture(std::string_view fileName, bool flipVertical = false);
};

} // namespace shady::utils
