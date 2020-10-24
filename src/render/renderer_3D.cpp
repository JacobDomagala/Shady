#include "render/renderer_3D.hpp"
#include "render/render_command.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"
#include "render/vertex.hpp"
#include "render/vertex_array.hpp"
#include "scene/camera.hpp"
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
   s_vertexBuffer = VertexBuffer::Create(s_maxVertices * sizeof(render::Vertex));
   s_vertexBuffer->SetLayout({{ShaderDataType::Float3, "a_Position"},
                                     {ShaderDataType::Float3, "a_Normal"},
                                     {ShaderDataType::Float2, "a_TexCoord"},
                                     {ShaderDataType::Float3, "a_Tangent"},
                                     {ShaderDataType::Float, "a_DiffTexIndex"},
                                     {ShaderDataType::Float, "a_NormTexIndex"},
                                     {ShaderDataType::Float, "a_SpecTexIndex"},
                                     {ShaderDataType::Mat4, "a_ModelMat"},
                                     {ShaderDataType::Float4, "a_Color"}});

   s_vertexArray->AddVertexBuffer(s_vertexBuffer);

   s_indexBuffer = IndexBuffer::Create(s_maxIndices);
   s_vertexArray->SetIndexBuffer(s_indexBuffer);

   s_verticesBatch.resize(s_maxVertices * sizeof(Vertex));
   s_indicesBatch.resize(s_maxIndices * sizeof(uint32_t));

   s_whiteTexture = TextureLibrary::GetTexture(TextureType::DIFFUSE_MAP, "white.png");

   s_textureShader = ShaderLibrary::GetShader("default");

   // Set all texture slots to 0
   s_textureSlots[0] = s_whiteTexture;

   trace::Logger::Info("Renderer3D::Init: shader:{} maxVertices:{} maxIndices:{} maxTextures:{}",
                       s_textureShader->GetName(), s_maxVertices, s_maxIndices,
                       s_maxTextureSlots);
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
Renderer3D::BeginScene(const scene::Camera& camera)
{
   trace::Logger::Debug("Renderer3D::BeginScene: shader:{}", s_textureShader->GetName());

   s_textureShader->Bind();
   s_textureShader->SetMat4("u_viewProjection", camera.GetViewProjection());

   int32_t samplers[s_maxTextureSlots];
   for (uint32_t i = 0; i < s_maxTextureSlots; i++)
   {
      samplers[i] = i;
   }

   s_textureShader->SetIntArray("u_Textures", samplers, s_maxTextureSlots);

   s_textureSlotIndex = 1;
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
   s_vertexBuffer->SetData(s_verticesBatch.data(),
                                  s_currentVertex * sizeof(Vertex));

   s_indexBuffer->SetData(s_indicesBatch.data(),
                                 s_currentIndex * sizeof(uint32_t));
   Flush();
}

void
Renderer3D::Flush()
{
   trace::Logger::Debug("Renderer3D::Flush: numVertices:{} ", s_currentVertex);

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

   RenderCommand::DrawIndexed(s_vertexArray, s_currentIndex);
}

void
Renderer3D::FlushAndReset()
{
   SendData();

   s_currentIndex = 0;
   s_currentVertex = 0;
   s_textureSlotIndex = 1;
}

void
Renderer3D::DrawMesh(std::vector< Vertex >& vertices, const std::vector< uint32_t >& indices,
                     const glm::vec3& translateVal, const glm::vec3& scaleVal,
                     const glm::vec3& rotateAxis, float radiansRotation,
                     const TexturePtrVec& textures, const glm::vec4& tintColor)
{
   time::ScopedTimer meshScope(fmt::format("Renderer3D::DrawMesh: numVertices:{} indices:{}",
                                           vertices.size(), indices.size()));

   if (s_currentVertex >= s_maxVertices
       || (s_textureSlotIndex + textures.size()) >= (s_maxTextureSlots - 1))
   {
      FlushAndReset();
   }

   auto meshTextures = SetupTextures(textures);

   trace::Logger::Debug(
      "DIFFUSE_MAP:{} SPECULAR_MAP:{} NORMAL_MAP:{}", meshTextures[TextureType::DIFFUSE_MAP],
      meshTextures[TextureType::SPECULAR_MAP], meshTextures[TextureType::NORMAL_MAP]);

   glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), translateVal)
                        * glm::rotate(glm::mat4(1.0f), radiansRotation, rotateAxis)
                        * glm::scale(glm::mat4(1.0f), scaleVal);

   std::transform(vertices.begin(), vertices.end(),
                  s_verticesBatch.begin() + s_currentVertex,
                  [&modelMat, &tintColor, &meshTextures](auto& vertex) {
                     vertex.m_color = tintColor;
                     vertex.m_modelMat = modelMat;
                     vertex.m_diffTexIndex = meshTextures[TextureType::DIFFUSE_MAP];
                     vertex.m_specTexIndex = meshTextures[TextureType::SPECULAR_MAP];
                     vertex.m_normTexIndex = meshTextures[TextureType::NORMAL_MAP];
                     return vertex;
                  });


   s_indicesBatch.insert(s_indicesBatch.begin() + s_currentIndex,
                                indices.begin(), indices.end());

   s_currentVertex += static_cast< uint32_t >(vertices.size());
   s_currentIndex += static_cast< uint32_t >(indices.size());
}

std::unordered_map< TextureType, float >
Renderer3D::SetupTextures(const TexturePtrVec& textures)
{
   if (textures.empty())
   {
      return {{TextureType::DIFFUSE_MAP, 0.0f},
              {TextureType::NORMAL_MAP, 0.0f},
              {TextureType::SPECULAR_MAP, 0.0f}};
   }

   std::unordered_map< TextureType, float > meshTextures;

   // Check whether textures for this mesh are already loaded
   for (auto& texture : textures)
   {
      float textureIndex = 0.0f;

      const auto textureIdxPtr =
         std::find_if(s_textureSlots.begin(), s_textureSlots.end(),
                      [&texture](const auto& textureSlot) {
                         return textureSlot ? *textureSlot == *texture : false;
                      });

      if (s_textureSlots.end() != textureIdxPtr)
      {
         textureIndex =
            static_cast< float >(std::distance(s_textureSlots.begin(), textureIdxPtr));
      }
      else
      {
         textureIndex = static_cast< float >(s_textureSlotIndex);
         s_textureSlots[s_textureSlotIndex] = texture;
         s_textureSlotIndex++;
      }

      meshTextures[texture->GetType()] = textureIndex;
   }

   return meshTextures;
}

} // namespace shady::render
