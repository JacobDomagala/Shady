#include "renderer_3D.hpp"
#include "camera.hpp"
#include "render_command.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex.hpp"
#include "vertex_array.hpp"

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
                                     {ShaderDataType::Float4, "a_Color"},
                                     {ShaderDataType::Float2, "a_TexCoord"},
                                     {ShaderDataType::Float, "a_TexIndex"},
                                     {ShaderDataType::Float, "a_TilingFactor"}});
   s_Data.m_vertexArray->AddVertexBuffer(s_Data.m_vertexBuffer);
   s_Data.m_vertexArray->SetIndexBuffer(IndexBuffer::Create(s_Data.m_maxIndices));

   s_Data.m_verticesBatch.reserve(s_Data.m_maxVertices * sizeof(Vertex));
   s_Data.m_indicesBatch.reserve(s_Data.m_maxIndices * sizeof(uint32_t));

   s_Data.m_whiteTexture = Texture::Create(glm::ivec2{1, 1});
   // s_Data.m_whiteTexture->CreateColorTexture({1, 1}, {1.0f, 1.0f, 1.0f});

   int32_t samplers[s_Data.m_maxTextureSlots];
   for (uint32_t i = 0; i < s_Data.m_maxTextureSlots; i++)
   {
      samplers[i] = i;
   }

   s_Data.m_textureShader = ShaderLibrary::GetShader("DefaultShader");
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
Renderer3D::BeginScene(const Camera& camera)
{
   s_Data.m_textureShader->Bind();
   s_Data.m_textureShader->SetMat4("u_ViewProjection", camera.GetViewProjection());

   s_Data.m_textureSlotIndex = 1;
}

void
Renderer3D::EndScene()
{
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
Renderer3D::DrawMesh(const std::vector< Vertex >& vertices, const std::vector< uint32_t >& indices,
                     const glm::vec3& translateVal, const glm::vec3& scaleVal,
                     float radiansRotation, const std::shared_ptr< Texture >& texture,
                     const glm::vec4& tintColor)
{
   if (s_Data.m_currentVertex >= RendererData::m_maxVertices)
   {
      FlushAndReset();
   }

   float textureIndex = 0.0f;

   // Check whether texture for this mesh is already loaded
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
      if (s_Data.m_textureSlotIndex >= RendererData::m_maxTextureSlots - 1)
      {
         FlushAndReset();
      }

      textureIndex = static_cast< float >(s_Data.m_textureSlotIndex);
      s_Data.m_textureSlots[s_Data.m_textureSlotIndex] = texture;
      s_Data.m_textureSlotIndex++;
   }

   glm::mat4 transformMat = glm::translate(glm::mat4(1.0f), translateVal)
                            * glm::rotate(glm::mat4(1.0f), radiansRotation, {0.0f, 0.0f, 1.0f})
                            * glm::scale(glm::mat4(1.0f), scaleVal);

   // for (auto& vertex : vertices)
   //{
   //   s_Data.m_vertexBufferPtr->m_position = transformMat * glm::vec4(vertex.m_position, 1.0f);
   //   s_Data.m_vertexBufferPtr->m_color = tintColor;
   //   s_Data.m_vertexBufferPtr->m_texCoords = vertex.m_texCoords;
   //   s_Data.m_vertexBufferPtr->m_texIndex = textureIndex;
   //   s_Data.m_vertexBufferPtr++;
   //}

   s_Data.m_verticesBatch.insert(s_Data.m_verticesBatch.begin() + s_Data.m_currentVertex,
                                 vertices.begin(), vertices.end());
   s_Data.m_indicesBatch.insert(s_Data.m_indicesBatch.begin() + s_Data.m_currentIndex,
                                indices.begin(), indices.end());

   s_Data.m_currentVertex += vertices.size();
   s_Data.m_currentIndex += indices.size();
}

} // namespace shady::render
