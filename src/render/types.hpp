#pragma once

#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <string>

namespace shady::render {

enum class TextureType : std::uint8_t
{
   DIFFUSE_MAP = 0,
   METALLIC_ROUGHNESS_MAP = 1,
   NORMAL_MAP = 2,
   CUBE_MAP = 3
};

struct UniformBufferObject
{
   glm::mat4 proj = {};
   glm::mat4 view = {};
   glm::mat4 lightView = {};
};

struct DebugData
{
   uint32_t displayDebugTarget = 0;
   int32_t pcfShadow = 1;
   float ambientLight = 0.1f;
   float shadowFactor = 0.1f;
};

struct PerInstanceBuffer
{
   glm::mat4 model = {};
   glm::ivec4 textures = glm::ivec4(-1);
   glm::vec4 baseColorFactor = glm::vec4(1.0F);
   glm::vec4 materialFactors = glm::vec4(1.0F, 1.0F, 1.0F, 0.0F);
};

// Base color, metallic-roughness, normal.
using TextureMaps = std::array< std::string, 3 >;

struct MaterialData
{
   TextureMaps textures = {};
   glm::vec4 baseColorFactor = glm::vec4(1.0F);
   float metallicFactor = 1.0F;
   float roughnessFactor = 1.0F;
   float normalScale = 1.0F;
};

} // namespace shady::render
