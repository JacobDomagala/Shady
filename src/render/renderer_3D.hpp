#pragma once

#include "render/texture.hpp"
#include "render/vertex.hpp"
#include "scene/model.hpp"

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
class DrawIndirectBuffer;
class ShaderStorageBuffer;
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
   AddMesh(const std::string& modelName, std::vector< Vertex >& vertices,
           const std::vector< uint32_t >& indices, const TexturePtrVec& textures = {});

   static void
   DrawMesh(const std::string& modelName, const glm::mat4& modelMat,
            const glm::vec4& tintColor = glm::vec4(1.0f));

 private:
   static void
   SendData();

   static void
   Flush();

   static void
   FlushAndReset();

   static std::unordered_map< TextureType, float >
   SetupTextures(const TexturePtrVec& textures);

   static float
   SetupModelMat(const std::string& modelName, const glm::mat4& modelMat);

 private:
   struct DrawElementsIndirectCommand
   {
      // number of indices for model
      uint32_t count;

      // number of instances to draw
      uint32_t instanceCount;

      // offset to first index in the currently bound index buffer
      uint32_t firstIndex;

      // offset to first vertex in the currently bound vertex buffer
      uint32_t baseVertex;

      // base draw instance
      uint32_t baseInstance;
   };

 private:
   // TODO: Figure out the proper way of setting triangle cap per batch
   static inline constexpr uint32_t s_maxTriangles = 100000;
   static inline constexpr uint32_t s_maxVertices = s_maxTriangles * 3;
   static inline constexpr uint32_t s_maxIndices = s_maxVertices * 2;
   static inline constexpr uint32_t s_maxTextureSlots = 32; // TODO: RenderCaps
   static inline constexpr uint32_t s_maxModelMatSlots = 32;

   static inline std::shared_ptr< VertexArray > s_vertexArray;
   static inline std::shared_ptr< VertexBuffer > s_vertexBuffer;
   static inline std::shared_ptr< IndexBuffer > s_indexBuffer;
   static inline std::shared_ptr< DrawIndirectBuffer > s_drawIndirectBuffer;
   static inline std::shared_ptr< ShaderStorageBuffer > s_ssbo;
   static inline std::shared_ptr< Shader > s_textureShader;
   static inline std::shared_ptr< Texture > s_whiteTexture;

   // data batches which are filled with each call to DrawMesh
   static inline std::vector< render::Vertex > s_verticesBatch;
   static inline uint32_t s_currentVertex = 0;
   static inline std::vector< uint32_t > s_indicesBatch;
   static inline uint32_t s_currentIndex = 0;

   static inline std::array< glm::mat4, s_maxModelMatSlots > s_modelMats;
   static inline uint32_t s_currentModelMatIdx = 0;
   static inline std::unordered_map< std::string, float > s_modelMatsIdx;

   static inline std::array< std::shared_ptr< Texture >, s_maxTextureSlots > s_textureSlots;
   static inline uint32_t s_textureSlotIndex = 1; // 0 = white texture

   static inline std::array< DrawElementsIndirectCommand, s_maxModelMatSlots > s_commands;
   static inline uint32_t s_numModels = 0;
};

} // namespace shady::render