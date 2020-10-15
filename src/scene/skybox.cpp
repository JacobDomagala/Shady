#include "skybox.hpp"
#include "render/render_command.hpp"
#include "render/texture.hpp"

#include <array>

namespace shady::scene {

void
Skybox::LoadCubeMap(const std::string& directoryPath)
{
   // Positions
   std::array< float, 32 > skyboxVertices = {
      -1.0f, 1.0f,  -1.0f, // vertex 0
      -1.0f, -1.0f, -1.0f, // vertex 1
      1.0f,  -1.0f, -1.0f, // vertex 2
      1.0f,  1.0f,  -1.0f, // vertex 3
      -1.0f, -1.0f, 1.0f,  // vertex 4
      -1.0f, 1.0f,  1.0f,  // vertex 5
      1.0f,  -1.0f, 1.0f,  // vertex 6
      1.0f,  1.0f,  1.0f   // vertex 7
   };

   auto vertexBuffer = render::VertexBuffer::Create(8);
   vertexBuffer->SetData(skyboxVertices.data(), sizeof(skyboxVertices));
   vertexBuffer->SetLayout(render::BufferLayout{{render::ShaderDataType::Float3, "a_Position"}});

   std::array< uint8_t, 24 > indicies = {
      0, 2, 1, 0, 3, 2, // face 1
      2, 3, 6, 3, 9, 6, // face 2
      7, 5, 4, 7, 4, 6, // face 3
      5, 1, 4, 5, 0, 1, // face 4
   };
   auto indexBuffer = render::IndexBuffer::Create(24);
   indexBuffer->SetData(indicies.data(), 24);

   m_vertexArray = render::VertexArray::Create();
   m_vertexArray->AddVertexBuffer(vertexBuffer);
   m_vertexArray->SetIndexBuffer(indexBuffer);

   m_cubeTexture = render::TextureLibrary::GetTexture(directoryPath, render::TextureType::CUBE_MAP);
   m_shader = render::ShaderLibrary::GetShader("Skybox");
}

void
Skybox::Draw(const render::Camera& camera)
{
   // We draw here directly without calling Renderer because we use unique shader
   // so we wouldn't be able to use batching anyways
   m_shader->Bind();

   render::RenderCommand::SetDepthFunc(render::DepthFunc::LEQUAL);

   m_shader->SetMat4("u_ViewProjectionMat", camera.GetViewProjection());
   m_vertexArray->Bind();
   m_cubeTexture->Bind(0);

   m_shader->SetInt("skybox", 0);

   render::RenderCommand::DrawIndexed(m_vertexArray);

   render::RenderCommand::SetDepthFunc(render::DepthFunc::LESS);
}

} // namespace shady::scene