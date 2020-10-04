#pragma once

#include "vertex.hpp"
#include "render/texture.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace shady::render {

class Camera;

class Renderer3D
{
 public:
   static void
   Init();

   static void
   Shutdown();

   static void
   BeginScene(const Camera& camera);

   static void
   EndScene();

   static void
   DrawMesh(const std::vector< Vertex >& vertices, const std::vector< uint32_t >& indices,
             const glm::vec3& translateVal = glm::vec3{0.0f},
             const glm::vec3& scaleVal = glm::vec3{1.0f}, float radiansRotation = 0.0f,
             const std::shared_ptr< Texture >& texture = Texture::Create(glm::ivec2{1,1}),
             const glm::vec4& tintColor = glm::vec4(1.0f));

 private:
   static void
   SendData();

   static void
   Flush();

   static void
   FlushAndReset();
};

} // namespace shady::render