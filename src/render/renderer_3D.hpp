#pragma once

#include "render/texture.hpp"
#include "render/vertex.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace shady::scene {
class Camera;
}

namespace shady::render {

class Renderer3D
{
 public:
   static void
   Init();

   static void
   Shutdown();

   static void
   BeginScene(const scene::Camera& camera);

   static void
   EndScene();

   static void
   DrawMesh(std::vector< Vertex >& vertices, const std::vector< uint32_t >& indices,
            const glm::vec3& translateVal = glm::vec3{0.0f},
            const glm::vec3& scaleVal = glm::vec3{1.0f},
            const glm::vec3& rotateAxis = glm::vec3{1.0f}, float radiansRotation = 0.0f,
            const TexturePtrVec& textures = {}, const glm::vec4& tintColor = glm::vec4(1.0f));

 private:
   static void
   SendData();

   static void
   Flush();

   static void
   FlushAndReset();

   static std::unordered_map< TextureType, float >
   SetupTextures(const TexturePtrVec& textures);
};

} // namespace shady::render