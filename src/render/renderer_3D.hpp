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
class Light;
} // namespace shady::scene

namespace shady::render {

class Shader;
class VertexArray;
class VertexBuffer;
class IndexBuffer;
class StorageBuffer;
class BufferLockManager;

class Renderer3D
{
 public:
   static void
   Init();

   static void
   Shutdown();

   static void
   BeginScene(const glm::ivec2& screenSize, const scene::Camera& camera, scene::Light& light,
              bool shadowMap = false);

   static void
   EndScene();

   static void
   AddMesh(const std::string& modelName, std::vector< Vertex >& vertices,
           const std::vector< uint32_t >& indices);

   static void
   DrawMesh(const std::string& modelName, const glm::mat4& modelMat, const TexturePtrVec& textures,
            const glm::vec4& tintColor = glm::vec4(1.0f));

 private:
   static void
   SendData();

   static void
   Flush();

   static void
   FlushAndReset();

   static std::unordered_map< TextureType, TextureHandleType >
   SetupTextures(const TexturePtrVec& textures);

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

   struct VertexBufferData
   {
      glm::mat4 modelMat;
      TextureHandleType diffuseTextureID;
      TextureHandleType normalTextureID;
      TextureHandleType specularTextureID;

      // padding
      uint64_t reserved;
   };

 private:
   // UPPER LIMITS
   static inline constexpr uint32_t s_maxTriangles = 500000;
   static inline constexpr uint32_t s_maxVertices = s_maxTriangles * 3;
   static inline constexpr uint32_t s_maxIndices = s_maxVertices * 2;
   static inline constexpr uint32_t s_maxMeshesSlots = 500;

   //-----------------------------------------
   // VAO/VBO
   static inline std::shared_ptr< VertexArray > s_vertexArray;
   static inline std::shared_ptr< VertexBuffer > s_drawIDs;
   static inline std::shared_ptr< VertexBuffer > s_vertexBuffer;
   static inline std::shared_ptr< IndexBuffer > s_indexBuffer;

   static inline std::vector< render::Vertex > s_verticesBatch;
   static inline uint32_t s_currentVertex = 0;
   static inline std::vector< uint32_t > s_indicesBatch;
   static inline uint32_t s_currentIndex = 0;

   //-----------------------------------------
   // SSBOs
   static inline std::shared_ptr< StorageBuffer > s_perMeshDataSSBO;
   static inline std::shared_ptr< Shader > s_textureShader;
   static inline std::shared_ptr< Shader > s_shadowShader;

   static inline std::vector< VertexBufferData > s_renderDataPerObj;

   static inline uint32_t s_numMeshes = 0;
   static inline uint32_t s_currentMeshIdx = 0;

   static inline TexturePtrVec s_textures;

   //-----------------------------------------
   // INDIRECT DATA
   static inline std::shared_ptr< StorageBuffer > s_drawIndirectBuffer;
   static inline std::vector< DrawElementsIndirectCommand > s_commands;
   static inline std::unordered_map< std::string, uint32_t > s_modelCommandMap;

   static inline std::shared_ptr< BufferLockManager > s_bufferLockManager;

   static inline bool s_sceneChanged = false;
   static inline bool s_drawingToShadowMap = false;
   static inline uint32_t s_screenWidth = 0;
   static inline uint32_t s_screenHeight = 0;
   static inline scene::Light* s_lightPtr;
};

} // namespace shady::render
