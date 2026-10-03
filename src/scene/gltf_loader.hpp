#pragma once

#include "render/types.hpp"
#include "render/vertex.hpp"

#include <array>
#include <string>
#include <tiny_gltf.h>
#include <vector>

namespace shady::scene {

// CPU-only import result: validation completes before any Vulkan resources are created.
struct GltfMesh
{
   std::string name;
   std::vector< render::Vertex > vertices;
   std::vector< uint32_t > indices;
   render::MaterialData material;
   std::array< int, 3 > images = {-1, -1, -1};
};

struct GltfAsset
{
   tinygltf::Model source;
   std::vector< GltfMesh > meshes;
   std::vector< std::string > warnings;
};

// Invalid or required unsupported data uses the engine's fatal assertion path.
[[nodiscard]] GltfAsset
LoadGltf(const std::string& path);
[[nodiscard]] GltfAsset
ImportGltf(tinygltf::Model model);

} // namespace shady::scene
