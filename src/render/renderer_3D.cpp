#include "render/renderer_3D.hpp"
#include "render/buffer_lock.hpp"
#include "render/render_command.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"
#include "render/vertex.hpp"
#include "render/vertex_array.hpp"
#include "scene/camera.hpp"
#include "scene/light.hpp"
#include "time/scoped_timer.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"
#include "utils/file_manager.hpp"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <numeric>


namespace shady::render {

GLuint depthMapFBO;
GLuint depthMap;
GLuint texColorBuffer;

void
Renderer3D::Init()
{
   s_verticesBatch.resize(s_maxVertices);
   s_indicesBatch.resize(s_maxIndices);

   s_commands.resize(s_maxMeshesSlots);
   s_renderDataPerObj.resize(s_maxMeshesSlots);

   s_textureShader = ShaderLibrary::GetShader("default");
   s_shadowShader = ShaderLibrary::GetShader(
      (utils::FileManager::SHADERS_DIR / "default" / "shadowmap_point").u8string());

   // @TODO: For now we hardcode size for 's_maxMeshesSlots' model matrices
   // which essentially means that we can store up to 's_maxMeshesSlots' unique model/meshes for
   // now
   s_perMeshDataSSBO =
      StorageBuffer::Create(BufferType::ShaderStorage, sizeof(VertexBufferData) * s_maxMeshesSlots);

   s_drawIndirectBuffer = StorageBuffer::Create(
      BufferType::DrawIndirect, sizeof(DrawElementsIndirectCommand) * s_maxMeshesSlots);

   trace::Logger::Info("Renderer3D::Init: shader:{} maxVertices:{} maxIndices:{}",
                       s_textureShader->GetName(), s_maxVertices, s_maxIndices);

   s_bufferLockManager = BufferLockManager::Create(true);
}

void
Renderer3D::Shutdown()
{
   s_vertexArray.reset();
   s_vertexBuffer.reset();
   s_textureShader.reset();
}

void
Renderer3D::BeginScene(const glm::ivec2& screenSize, const scene::Camera& camera,
                       scene::Light& light, bool shadowMap)
{
   trace::Logger::Debug("Renderer3D::BeginScene: {} shader:{}",
                        shadowMap ? "FIRST_PASS" : "SECOND_PASS",
                        shadowMap ? s_shadowShader->GetName() : s_textureShader->GetName());

   s_screenWidth = screenSize.x;
   s_screenHeight = screenSize.y;
   s_drawingToShadowMap = shadowMap;
   s_lightPtr = &light;

   if (!s_drawingToShadowMap)
   {
      RenderCommand::SetViewport(0, 0, s_screenWidth, s_screenHeight);

      s_textureShader->Bind();
      // @TODO: Mby use uniform buffer for all these?
      s_textureShader->SetMat4("u_lightSpaceMat", light.GetLightSpaceMat());
      s_textureShader->SetFloat3("u_lightPos", light.GetPosition());
      s_textureShader->SetMat4("u_projectionMat", camera.GetProjection());
      s_textureShader->SetMat4("u_viewMat", camera.GetView());
      s_textureShader->SetFloat3("u_cameraPos", camera.GetPosition());

      light.BindLightMap(0);
      s_textureShader->SetInt("u_depthMap", 0);
   }
   else
   {
      const auto shadowMapSize = light.GetLightmapSize();
      RenderCommand::SetViewport(0, 0, shadowMapSize.x, shadowMapSize.y);

      light.BeginRenderToLightmap();

      s_shadowShader->Bind();
      s_shadowShader->SetMat4("u_lightSpaceMat", light.GetLightSpaceMat());
   }

   if (s_sceneChanged)
   {
      s_vertexArray = VertexArray::Create();

      // setup vertex buffer
      s_vertexBuffer = VertexBuffer::Create(reinterpret_cast< float* >(s_verticesBatch.data()),
                                            s_maxVertices * sizeof(render::Vertex));
      s_vertexBuffer->SetLayout({{ShaderDataType::Float3, "a_position"},
                                 {ShaderDataType::Float3, "a_normal"},
                                 {ShaderDataType::Float2, "a_texCoord"},
                                 {ShaderDataType::Float3, "a_tangent"},
                                 {ShaderDataType::Float4, "a_color"}});

      s_vertexArray->AddVertexBuffer(s_vertexBuffer);


      std::array< float, s_maxMeshesSlots > drawIDs;
      std::iota(std::begin(drawIDs), std::end(drawIDs), 0);
      s_drawIDs = VertexBuffer::Create(drawIDs.data(), s_maxMeshesSlots * sizeof(float));
      s_drawIDs->SetLayout({{ShaderDataType::Float, "a_drawID", 1}});

      s_vertexArray->AddVertexBuffer(s_drawIDs);

      s_indexBuffer = IndexBuffer::Create(s_indicesBatch.data(), s_maxIndices);
      s_vertexArray->SetIndexBuffer(s_indexBuffer);

      // s_bufferLockManager->WaitForLockedRange(0, size);
      // s_vertexBuffer->SetData(s_verticesBatch.data(), size);
      // s_bufferLockManager->LockRange(0, size);

      // s_indexBuffer->SetData(s_indicesBatch.data(), s_currentIndex * sizeof(uint32_t));

      s_sceneChanged = false;
   }
}

void
Renderer3D::EndScene()
{
   trace::Logger::Debug("Renderer3D::EndScene: {}",
                        s_drawingToShadowMap ? "FIRST_PASS" : "SECOND_PASS");
   FlushAndReset();
}

void
Renderer3D::SendData()
{
   Flush();
}

void
Renderer3D::Flush()
{
   // SCOPED_TIMER(fmt::format("Renderer3D::Flush: numMeshes:{} numVertices:{} numTextures:{}",
   //                          s_numMeshes, s_currentVertex, s_textures.size()));

   if (s_currentVertex == 0)
   {
      return; // Nothing to draw
   }

   s_vertexArray->Bind();

   const auto bufferDataSSBOSize = s_maxMeshesSlots * sizeof(VertexBufferData);
   const auto elemsIndirectSize = s_maxMeshesSlots * sizeof(DrawElementsIndirectCommand);

   s_perMeshDataSSBO->SetData(s_renderDataPerObj.data(), bufferDataSSBOSize);
   s_perMeshDataSSBO->BindBufferRange(0, bufferDataSSBOSize);

   s_drawIndirectBuffer->SetData(s_commands.data(), elemsIndirectSize);

   RenderCommand::MultiDrawElemsIndirect(s_numMeshes, s_drawIndirectBuffer->GetOffset());

   s_perMeshDataSSBO->OnUsageComplete(bufferDataSSBOSize);
   s_drawIndirectBuffer->OnUsageComplete(elemsIndirectSize);

   s_currentMeshIdx = 0;

   for (auto& texture : s_textures)
   {
      texture->MakeNonResident();
   }
   s_textures.clear();

   if (s_drawingToShadowMap)
   {
      s_lightPtr->EndRenderToLightmap();
   }
}

void
Renderer3D::FlushAndReset()
{
   SendData();
}

void
Renderer3D::DrawMesh(const std::string& modelName, const glm::mat4& modelMat,
                     const TexturePtrVec& textures, const glm::vec4& tintColor)
{
   s_renderDataPerObj[s_currentMeshIdx].modelMat = modelMat;

   auto meshTextures = SetupTextures(textures);

   s_renderDataPerObj[s_currentMeshIdx].diffuseTextureID = meshTextures[TextureType::DIFFUSE_MAP];
   s_renderDataPerObj[s_currentMeshIdx].normalTextureID = meshTextures[TextureType::NORMAL_MAP];
   s_renderDataPerObj[s_currentMeshIdx].specularTextureID = meshTextures[TextureType::SPECULAR_MAP];

   s_commands[s_modelCommandMap[modelName]].baseInstance = s_currentMeshIdx;
   ++s_currentMeshIdx;
}

void
Renderer3D::AddMesh(const std::string& modelName, std::vector< Vertex >& vertices,
                    const std::vector< uint32_t >& indices)
{
   s_verticesBatch.insert(s_verticesBatch.begin() + s_currentVertex, vertices.begin(),
                          vertices.end());

   s_indicesBatch.insert(s_indicesBatch.begin() + s_currentIndex, indices.begin(), indices.end());

   DrawElementsIndirectCommand newModel;
   newModel.count = static_cast< uint32_t >(indices.size());
   newModel.instanceCount = 1;
   newModel.firstIndex = s_currentIndex;
   newModel.baseVertex = s_currentVertex;
   newModel.baseInstance = 0;

   s_commands[s_numMeshes] = newModel;
   s_modelCommandMap[modelName] = s_numMeshes;
   ++s_numMeshes;

   s_currentVertex += static_cast< uint32_t >(vertices.size());
   s_currentIndex += static_cast< uint32_t >(indices.size());

   s_sceneChanged = true;
}

std::unordered_map< TextureType, TextureHandleType >
Renderer3D::SetupTextures(const TexturePtrVec& textures)
{
   if (textures.empty())
   {
      return {{TextureType::DIFFUSE_MAP, 0},
              {TextureType::NORMAL_MAP, 0},
              {TextureType::SPECULAR_MAP, 0}};
   }

   std::unordered_map< TextureType, TextureHandleType > meshTextures;

   for (auto& texture : textures)
   {
      const auto texHandle = texture->GetTextureHandle();

      auto iter = std::find_if(s_textures.begin(), s_textures.end(),
                               [&texture](const auto& tex) { return *texture == *tex; });

      if (iter == s_textures.end())
      {
         s_textures.push_back(texture);
         texture->MakeResident();
      }

      meshTextures[texture->GetType()] = texHandle;
   }

   return meshTextures;
}

} // namespace shady::render
