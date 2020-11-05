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


#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace shady::render {

void
Renderer3D::Init()
{
   s_vertexArray = VertexArray::Create();

   // setup vertex buffer
   s_vertexBuffer = VertexBuffer::CreatePersistanceMap(s_maxVertices * sizeof(render::Vertex));
   s_vertexBuffer->SetLayout({{ShaderDataType::Float3, "a_position"},
                              {ShaderDataType::Float3, "a_normal"},
                              {ShaderDataType::Float2, "a_texCoord"},
                              {ShaderDataType::Float3, "a_tangent"},
                              {ShaderDataType::Float4, "a_color"}});

   s_vertexArray->AddVertexBuffer(s_vertexBuffer);

   s_indexBuffer = IndexBuffer::Create(s_maxIndices);
   s_vertexArray->SetIndexBuffer(s_indexBuffer);

   s_verticesBatch.resize(s_maxVertices);
   s_indicesBatch.resize(s_maxIndices);
   s_commands.resize(s_maxModelMatSlots);
   s_renderDataPerObj.resize(s_maxModelMatSlots);

   s_whiteTexture = TextureLibrary::GetTexture(TextureType::DIFFUSE_MAP, "white.png");
   s_textureShader = ShaderLibrary::GetShader("default");

   s_textureSlots[0] = s_whiteTexture;

   // @TODO: For now we hardcode size for 's_maxModelMatSlots' model matrices
   // which essentially means that we can store up to 's_maxModelMatSlots' unique model/meshes for
   // now
   s_ssbo = StorageBuffer::Create(BufferType::ShaderStorage,
                                  sizeof(VertexBufferData) * s_maxModelMatSlots);
   s_drawIndirectBuffer = StorageBuffer::Create(
      BufferType::DrawIndirect, sizeof(DrawElementsIndirectCommand) * s_maxModelMatSlots);

   trace::Logger::Info("Renderer3D::Init: shader:{} maxVertices:{} maxIndices:{} maxTextures:{}",
                       s_textureShader->GetName(), s_maxVertices, s_maxIndices, s_maxTextureSlots);

   s_bufferLockManager = BufferLockManager::Create(true);
}

void
Renderer3D::Shutdown()
{
   s_vertexArray.reset();
   s_vertexBuffer.reset();
   s_whiteTexture.reset();

   // TextureLibrary::Clear();
   // ShaderLibrary::Clear();

   s_textureShader.reset();
}

void
Renderer3D::BeginScene(const scene::Camera& camera, const scene::Light& light)
{
   trace::Logger::Debug("Renderer3D::BeginScene: shader:{}", s_textureShader->GetName());

   s_textureShader->Bind();
   s_textureShader->SetMat4("u_projectionMat", camera.GetProjection());
   s_textureShader->SetMat4("u_viewMat", camera.GetView());
   s_textureShader->SetFloat3("u_cameraPos", camera.GetPosition());
   s_textureShader->SetFloat3("u_lightPos", light.GetPosition());

   int32_t samplers[s_maxTextureSlots];
   for (uint32_t i = 0; i < s_maxTextureSlots; i++)
   {
      samplers[i] = i;
   }

   s_textureShader->SetIntArray("u_textures", samplers, s_maxTextureSlots);

   const auto size = s_currentVertex * sizeof(Vertex);

   if (s_sceneChanged)
   {
      s_bufferLockManager->WaitForLockedRange(0, size);
      s_vertexBuffer->SetData(s_verticesBatch.data(), size);
      s_bufferLockManager->LockRange(0, size);

      s_indexBuffer->SetData(s_indicesBatch.data(), s_currentIndex * sizeof(uint32_t));

      s_sceneChanged = false;
   }

   // s_textureSlotIndex = 1;
}

void
Renderer3D::EndScene()
{
   trace::Logger::Debug("Renderer3D::EndScene");
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
   SCOPED_TIMER(fmt::format(
      "Renderer3D::Flush: numMeshes:{} numVertices:{} numTextures:{}",
      s_numModels, s_currentVertex, s_textureSlotIndex));

   if (s_currentVertex == 0)
   {
      return; // Nothing to draw
   }

   s_vertexArray->Bind();
   s_textureShader->Bind();

   // Bind textures
   for (uint32_t i = 0; i < s_textureSlotIndex; i++)
   {
      s_textureSlots[i]->Bind(i);
   }

   const auto modelMatsSize = s_renderDataPerObj.size() * sizeof(VertexBufferData);
   const auto elemsIndirectSize = s_numModels * sizeof(DrawElementsIndirectCommand);

   s_ssbo->SetData(s_renderDataPerObj.data(), modelMatsSize);
   s_drawIndirectBuffer->SetData(s_commands.data(), elemsIndirectSize);

   s_ssbo->BindBufferRange(0, modelMatsSize);

   RenderCommand::MultiDrawElemsIndirect(s_numModels, s_drawIndirectBuffer->GetOffset());

   s_ssbo->OnUsageComplete(modelMatsSize);
   s_drawIndirectBuffer->OnUsageComplete(elemsIndirectSize);

   s_currentModelIdx = 0;
}

void
Renderer3D::FlushAndReset()
{
   SendData();

   // s_currentIndex = 0;
   // s_currentVertex = 0;
   // s_textureSlotIndex = 1;
   // s_currentModelMatIdx = 0;
}

void
Renderer3D::DrawMesh(const std::string& modelName, const glm::mat4& modelMat,
                     const TexturePtrVec& textures, const glm::vec4& tintColor)
{
   // auto modelMatIdx = SetupModelMat(modelName, modelMat);
   s_renderDataPerObj[s_currentModelIdx].modelMat = modelMat;

   auto meshTextures = SetupTextures(textures);
   s_renderDataPerObj[s_currentModelIdx].diffuseTextureID = meshTextures[TextureType::DIFFUSE_MAP];
   s_renderDataPerObj[s_currentModelIdx].normalTextureID = meshTextures[TextureType::NORMAL_MAP];
   s_renderDataPerObj[s_currentModelIdx].specularTextureID =
      meshTextures[TextureType::SPECULAR_MAP];

   ++s_currentModelIdx;
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

   s_commands[s_numModels] = newModel;
   s_modelCommandMap[modelName] = s_numModels;
   ++s_numModels;

   s_currentVertex += static_cast< uint32_t >(vertices.size());
   s_currentIndex += static_cast< uint32_t >(indices.size());

   s_sceneChanged = true;
}

std::unordered_map< TextureType, int32_t >
Renderer3D::SetupTextures(const TexturePtrVec& textures)
{
   if (textures.empty())
   {
      return {{TextureType::DIFFUSE_MAP, 0.0f},
              {TextureType::NORMAL_MAP, 0.0f},
              {TextureType::SPECULAR_MAP, 0.0f}};
   }

   std::unordered_map< TextureType, int32_t > meshTextures;

   // Check whether textures for this mesh are already loaded
   for (auto& texture : textures)
   {
      int32_t textureIndex = 0;

      const auto textureIdxPtr = std::find_if(
         s_textureSlots.begin(), s_textureSlots.end(), [&texture](const auto& textureSlot) {
            return textureSlot ? *textureSlot == *texture : false;
         });

      if (s_textureSlots.end() != textureIdxPtr)
      {
         textureIndex = std::distance(s_textureSlots.begin(), textureIdxPtr);
      }
      else
      {
         textureIndex = s_textureSlotIndex;
         s_textureSlots[s_textureSlotIndex] = texture;
         s_textureSlotIndex++;
      }

      meshTextures[texture->GetType()] = textureIndex;
   }


   return meshTextures;
}

} // namespace shady::render
