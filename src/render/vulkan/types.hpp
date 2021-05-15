#pragma once

#include <array>
#include <string>

namespace shady::render::vulkan {

enum class TextureType
{
   DIFFUSE_MAP = 0,
   SPECULAR_MAP = 1,
   NORMAL_MAP = 2,
   CUBE_MAP = 3
};

// DIFFUSE_MAP SPECULAR_MAP NORMAL_MAP
using TextureMaps = std::array< std::string, 3 >;

} // namespace shady::render::vulkan
