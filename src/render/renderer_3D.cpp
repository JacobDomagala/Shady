#include "render/renderer_3D.hpp"
#include "scene/camera.hpp"
#include "render/render_command.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"
#include "render/vertex.hpp"
#include "render/vertex_array.hpp"
#include "trace/logger.hpp"

#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace shady::render {

struct RendererData
{
   // TODO: Figure out the proper way of setting triangle cap per batch
   static constexpr uint32_t m_maxTriangles = 20000;
   static constexpr uint32_t m_maxVertices = m_maxTriangles * 3;
   static constexpr uint32_t m_maxIndices = m_maxVertices;
   static constexpr uint32_t m_maxTextureSlots = 32; // TODO: RenderCaps

   std::shared_ptr< VertexArray > m_vertexArray;
   std::shared_ptr< VertexBuffer > m_vertexBuffer;
   std::shared_ptr< Shader > m_textureShader;
   std::shared_ptr< Texture > m_whiteTexture;

   // data batches which are filled with each call to DrawMesh
   std::vector< render::Vertex > m_verticesBatch;
   uint32_t m_currentVertex = 0;
   std::vector< uint32_t > m_indicesBatch;
   uint32_t m_currentIndex = 0;

   std::array< std::shared_ptr< Texture >, m_maxTextureSlots > m_textureSlots;
   uint32_t m_textureSlotIndex = 1; // 0 = white texture
};

static RendererData s_Data;

void
Renderer3D::Init()
{
   s_Data.m_vertexArray = VertexArray::Create();

   // setup vertex buffer
   s_Data.m_vertexBuffer = VertexBuffer::Create(s_Data.m_maxVertices * sizeof(render::Vertex));
   s_Data.m_vertexBuffer->SetLayout({{ShaderDataType::Float3, "a_Position"},
                                     {ShaderDataType::Float3, "a_Normal"},
                                     {ShaderDataType::Float2, "a_TexCoord"},
                                     {ShaderDataType::Float3, "a_Tangent"},
                                     {ShaderDataType::Float, "a_DiffTexIndex"},
                                     {ShaderDataType::Float, "a_NormTexIndex"},
                                     {ShaderDataType::Float, "a_SpecTexIndex"},
                                     {ShaderDataType::Float4, "a_Color"}});

   s_Data.m_vertexArray->AddVertexBuffer(s_Data.m_vertexBuffer);
   s_Data.m_vertexArray->SetIndexBuffer(IndexBuffer::Create(s_Data.m_maxIndices));

   s_Data.m_verticesBatch.reserve(s_Data.m_maxVertices * sizeof(Vertex));
   s_Data.m_indicesBatch.reserve(s_Data.m_maxIndices * sizeof(uint32_t));

   s_Data.m_whiteTexture = Texture::Create(TextureType::DIFFUSE_MAP, glm::ivec2{1, 1});
   // s_Data.m_whiteTexture->CreateColorTexture({1, 1}, {1.0f, 1.0f, 1.0f});

   int32_t samplers[s_Data.m_maxTextureSlots];
   for (uint32_t i = 0; i < s_Data.m_maxTextureSlots; i++)
   {
      samplers[i] = i;
   }

   s_Data.m_textureShader = ShaderLibrary::GetShader("default");
   s_Data.m_textureShader->Bind();
   s_Data.m_textureShader->SetIntArray("u_Textures", samplers, s_Data.m_maxTextureSlots);

   // Set all texture slots to 0
   s_Data.m_textureSlots[0] = s_Data.m_whiteTexture;
}

void
Renderer3D::Shutdown()
{
   s_Data.m_vertexArray.reset();
   s_Data.m_vertexBuffer.reset();
   s_Data.m_whiteTexture.reset();

   // TextureLibrary::Clear();
   // ShaderLibrary::Clear();

   s_Data.m_textureShader.reset();
}

void
Renderer3D::BeginScene(const scene::Camera& camera)
{
   trace::Logger::Info("Renderer3D::BeginScene: shader:{}", s_Data.m_textureShader->GetName());

   s_Data.m_textureShader->Bind();
   s_Data.m_textureShader->SetMat4("u_ViewProjection", camera.GetViewProjection());

   s_Data.m_textureSlotIndex = 1;
}

void
Renderer3D::EndScene()
{
   trace::Logger::Info("Renderer3D::EndScene");
   SendData();
}

void
Renderer3D::SendData()
{
   s_Data.m_vertexBuffer->SetData(s_Data.m_verticesBatch.data(), s_Data.m_currentVertex);
   Flush();
}

void
Renderer3D::Flush()
{
   trace::Logger::Info("Renderer3D::Flush: numVertices:{} ", s_Data.m_currentVertex);

   if (s_Data.m_currentVertex == 0)
   {
      return; // Nothing to draw
   }

   s_Data.m_vertexArray->Bind();
   s_Data.m_vertexBuffer->Bind();
   s_Data.m_textureShader->Bind();

   // Bind textures
   for (uint32_t i = 0; i < s_Data.m_textureSlotIndex; i++)
   {
      s_Data.m_textureSlots[i]->Bind(i);
   }

   RenderCommand::DrawIndexed(s_Data.m_vertexArray, s_Data.m_currentIndex);
}

void
Renderer3D::FlushAndReset()
{
   SendData();

   s_Data.m_currentIndex = 0;
   s_Data.m_currentVertex = 0;
   s_Data.m_textureSlotIndex = 1;
}

void
Renderer3D::DrawMesh(std::vector< Vertex >& vertices, const std::vector< uint32_t >& indices,
                     const glm::vec3& translateVal, const glm::vec3& scaleVal,
                     const glm::vec3& rotateAxis, float radiansRotation,
                     const TexturePtrVec& textures, const glm::vec4& tintColor)
{
   if (s_Data.m_currentVertex >= RendererData::m_maxVertices
       || (s_Data.m_textureSlotIndex + textures.size()) >= (RendererData::m_maxTextureSlots - 1))
   {
      FlushAndReset();
   }

   auto meshTextures = SetupTextures(textures);

   glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), translateVal)
                        * glm::rotate(glm::mat4(1.0f), radiansRotation, rotateAxis)
                        * glm::scale(glm::mat4(1.0f), scaleVal);


   for (auto& vertex : vertices)
   {
      vertex.m_color = tintColor;
      vertex.m_position = modelMat * glm::vec4(vertex.m_position, 1.0f);
      vertex.m_diffTexIndex = meshTextures[TextureType::DIFFUSE_MAP];
      vertex.m_specTexIndex = meshTextures[TextureType::SPECULAR_MAP];
      vertex.m_normTexIndex = meshTextures[TextureType::NORMAL_MAP];
   }

   s_Data.m_verticesBatch.insert(s_Data.m_verticesBatch.begin() + s_Data.m_currentVertex,
                                 vertices.begin(), vertices.end());
   s_Data.m_indicesBatch.insert(s_Data.m_indicesBatch.begin() + s_Data.m_currentIndex,
                                indices.begin(), indices.end());

   s_Data.m_currentVertex += static_cast< uint32_t >(vertices.size());
   s_Data.m_currentIndex += static_cast< uint32_t >(indices.size());
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
         std::find_if(s_Data.m_textureSlots.begin(), s_Data.m_textureSlots.end(),
                      [&texture](const auto& textureSlot) { return *textureSlot == *texture; });

      if (s_Data.m_textureSlots.end() != textureIdxPtr)
      {
         textureIndex =
            static_cast< float >(std::distance(s_Data.m_textureSlots.begin(), textureIdxPtr));
      }
      else
      {
         textureIndex = static_cast< float >(s_Data.m_textureSlotIndex);
         s_Data.m_textureSlots[s_Data.m_textureSlotIndex] = texture;
         s_Data.m_textureSlotIndex++;
      }

      meshTextures[texture->GetType()] = textureIndex;
   }

   return meshTextures;
}

} // namespace shady::render
