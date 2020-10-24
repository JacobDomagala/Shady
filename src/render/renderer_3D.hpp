#pragma once

#include "render/texture.hpp"
#include "render/vertex.hpp"

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <vector>


namespace shady::scene {
class Camera;
}

namespace shady::render {

class Shader;
class Texture;
class VertexArray;
class VertexBuffer;
class IndexBuffer;

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

 private:
   // TODO: Figure out the proper way of setting triangle cap per batch
   static inline const uint32_t s_maxTriangles = 1000;
   static inline const uint32_t s_maxVertices = s_maxTriangles * 3;
   static inline const uint32_t s_maxIndices = s_maxVertices * 2;
   static inline const uint32_t s_maxTextureSlots = 32; // TODO: RenderCaps

   static inline std::shared_ptr< VertexArray > s_vertexArray;
   static inline std::shared_ptr< VertexBuffer > s_vertexBuffer;
   static inline std::shared_ptr< IndexBuffer > s_indexBuffer;
   static inline std::shared_ptr< Shader > s_textureShader;
   static inline std::shared_ptr< Texture > s_whiteTexture;

   // data batches which are filled with each call to DrawMesh
   static inline std::vector< render::Vertex > s_verticesBatch;
   static inline uint32_t s_currentVertex = 0;
   static inline std::vector< uint32_t > s_indicesBatch;
   static inline uint32_t s_currentIndex = 0;

   static inline std::array< std::shared_ptr< Texture >, s_maxTextureSlots > s_textureSlots;
   static inline uint32_t s_textureSlotIndex = 1; // 0 = white texture
};

} // namespace shady::render