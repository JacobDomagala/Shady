#pragma once

#include <array>
#include <string>
#include <glm/glm.hpp>

namespace shady::render {

enum class TextureType
{
   DIFFUSE_MAP = 0,
   SPECULAR_MAP = 1,
   NORMAL_MAP = 2,
   CUBE_MAP = 3
};

struct alignas(128) UniformBufferObject
{
   glm::mat4 proj = {};
   glm::mat4 view = {};
   glm::mat4 lightView = {};
};

struct alignas(16) DebugData
{
   uint32_t displayDebugTarget = 0;
   int32_t pcfShadow = 1;
   float ambientLight = 0.1f;
   float shadowFactor = 0.1f;
};

struct alignas(128) PerInstanceBuffer
{
   glm::mat4 model = {};
   glm::vec4 textures = {};
};

// DIFFUSE_MAP SPECULAR_MAP NORMAL_MAP
using TextureMaps = std::array< std::string, 3 >;

} // namespace shady::render
