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
   s_commands.resize(s_maxMeshesSlots);
   s_renderDataPerObj.resize(s_maxMeshesSlots);

   s_whiteTexture = TextureLibrary::GetTexture(TextureType::DIFFUSE_MAP, "white.png");
   s_textureShader = ShaderLibrary::GetShader("default");

   // @TODO: For now we hardcode size for 's_maxMeshesSlots' model matrices
   // which essentially means that we can store up to 's_maxMeshesSlots' unique model/meshes for
   // now
   s_transforms =
      StorageBuffer::Create(BufferType::ShaderStorage, sizeof(VertexBufferData) * s_maxMeshesSlots);

   s_drawIndirectBuffer = StorageBuffer::Create(
      BufferType::DrawIndirect, sizeof(DrawElementsIndirectCommand) * s_maxMeshesSlots);

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

   const auto size = s_currentVertex * sizeof(Vertex);

   if (s_sceneChanged)
   {
      s_bufferLockManager->WaitForLockedRange(0, size);
      s_vertexBuffer->SetData(s_verticesBatch.data(), size);
      s_bufferLockManager->LockRange(0, size);

      s_indexBuffer->SetData(s_indicesBatch.data(), s_currentIndex * sizeof(uint32_t));

      s_sceneChanged = false;
   }
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
   SCOPED_TIMER(fmt::format("Renderer3D::Flush: numMeshes:{} numVertices:{} numTextures:{}",
                            s_numMeshes, s_currentVertex, s_textures.size()));

   if (s_currentVertex == 0)
   {
      return; // Nothing to draw
   }

   s_vertexArray->Bind();
   s_textureShader->Bind();

   const auto bufferDataSSBOSize = s_renderDataPerObj.size() * sizeof(VertexBufferData);
   // const auto textureDataSSBOSize = s_textureDataPerObj.size() * sizeof(TexAddress);
   const auto elemsIndirectSize = s_numMeshes * sizeof(DrawElementsIndirectCommand);

   s_transforms->SetData(s_renderDataPerObj.data(), bufferDataSSBOSize);
   s_transforms->BindBufferRange(0, bufferDataSSBOSize);

   s_drawIndirectBuffer->SetData(s_commands.data(), elemsIndirectSize);

   RenderCommand::MultiDrawElemsIndirect(s_numMeshes, s_drawIndirectBuffer->GetOffset());

   s_transforms->OnUsageComplete(bufferDataSSBOSize);
   s_drawIndirectBuffer->OnUsageComplete(elemsIndirectSize);

   s_currentMeshIdx = 0;

   for (auto& texture : s_textures)
   {
      texture->MakeNonResident();
   }
   s_textures.clear();
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
