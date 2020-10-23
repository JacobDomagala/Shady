#include "skybox.hpp"
#include "render/render_command.hpp"
#include "render/texture.hpp"
#include "utils/file_manager.hpp"

#include <array>

namespace shady::scene {

void
Skybox::LoadCubeMap(const std::string& directoryPath)
{
   // Positions
   std::array< float, 24 > skyboxVertices = {
      -1.0f, 1.0f,  1.0f, // vertex 0
      -1.0f, -1.0f, -1.0f,  // vertex 1
      1.0f,  -1.0f, 1.0f, // vertex 2
      1.0f,  1.0f,  1.0f, // vertex 3
      -1.0f, -1.0f, -1.0f,  // vertex 4
      -1.0f, 1.0f,  -1.0f,  // vertex 5
      1.0f,  -1.0f, -1.0f,  // vertex 6
      1.0f,  1.0f,  -1.0f   // vertex 7
   };

   auto vertexBuffer =
      render::VertexBuffer::Create(skyboxVertices.data(), skyboxVertices.size() * sizeof(float));
   vertexBuffer->SetLayout({{render::ShaderDataType::Float3, "a_Position"}});

   std::array< uint32_t, 36 > indicies = {
      0, 2, 3, 1, 2, 0, // face 1
      3, 2, 6, 3, 6, 7, // face 2
      4, 7, 6, 4, 5, 7, // face 3
      1, 0, 4, 4, 0, 5, // face 4
      0, 3, 7, 0, 7, 5, // face 5
      4, 6, 2, 4, 2, 1  // face 6
   };
   auto indexBuffer = render::IndexBuffer::Create(indicies.data(), indicies.size());

   m_vertexArray = render::VertexArray::Create();
   m_vertexArray->AddVertexBuffer(vertexBuffer);
   m_vertexArray->SetIndexBuffer(indexBuffer);

   m_cubeTexture = render::TextureLibrary::GetTexture(render::TextureType::CUBE_MAP, directoryPath);
   m_shader = render::ShaderLibrary::GetShader(
      (utils::FileManager::SHADERS_DIR / "cubemapenv" / "skybox").u8string());
}

void
Skybox::Draw(const Camera& camera)
{
   // We draw here directly without calling Renderer because we use unique shader
   // so we wouldn't be able to use batching anyways
   m_shader->Bind();

   render::RenderCommand::SetDepthFunc(render::DepthFunc::LEQUAL);

   m_shader->SetMat4("u_viewProjection", camera.GetViewProjection());
   m_vertexArray->Bind();
   m_vertexArray->GetVertexBuffers().front()->Bind();
   m_vertexArray->GetIndexBuffer()->Bind();
   m_cubeTexture->Bind(0);

   m_shader->SetInt("skybox", 0);

   render::RenderCommand::DrawIndexed(m_vertexArray);

   render::RenderCommand::SetDepthFunc(render::DepthFunc::LESS);
}

} // namespace shady::scene