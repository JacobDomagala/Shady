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
      -1.0f, 1.0f,  1.0f,  // vertex 0
      -1.0f, -1.0f, 1.0f,  // vertex 1
      1.0f,  -1.0f, 1.0f,  // vertex 2
      1.0f,  1.0f,  1.0f,  // vertex 3
      -1.0f, -1.0f, -1.0f, // vertex 4
      -1.0f, 1.0f,  -1.0f, // vertex 5
      1.0f,  -1.0f, -1.0f, // vertex 6
      1.0f,  1.0f,  -1.0f  // vertex 7
   };

   auto vertexBuffer =
      render::VertexBuffer::Create(skyboxVertices.data(), skyboxVertices.size() * sizeof(float));
   vertexBuffer->SetLayout(shady::render::BufferLayout{{render::ShaderDataType::Float3, "a_Position"}});

   std::array< uint32_t, 36 > indicies = {
      3, 2, 0, 0, 2, 1, // face 1
      6, 2, 3, 7, 6, 3, // face 2
      6, 7, 4, 7, 5, 4, // face 3
      4, 0, 1, 5, 0, 4, // face 4
      7, 3, 0, 5, 7, 0, // face 5
      2, 6, 4, 1, 2, 4  // face 6
   };
   auto indexBuffer = render::IndexBuffer::Create(indicies.data(), static_cast<uint32_t>(indicies.size()));

   m_vertexArray = render::VertexArray::Create();
   m_vertexArray->AddVertexBuffer(vertexBuffer);
   m_vertexArray->SetIndexBuffer(indexBuffer);

   m_cubeTexture = render::TextureLibrary::GetTexture(render::TextureType::CUBE_MAP, directoryPath);
   m_shader = render::ShaderLibrary::GetShader(
      (utils::FileManager::SHADERS_DIR / "cubemapenv" / "skybox").u8string());
}

void
Skybox::Draw(const Camera& camera, uint32_t windowWidth, uint32_t windowHeight)
{
   render::RenderCommand::SetViewport(0, 0, windowWidth, windowHeight);

   // We draw here directly without calling Renderer because we use unique shader
   // so we wouldn't be able to use batching anyways
   m_shader->Bind();

   render::RenderCommand::DisableDepthTesting();

   // cutoff camera position part
   m_shader->SetMat4("u_viewProjection",
                     camera.GetProjection() * glm::mat4(glm::mat3(camera.GetView())));
   m_vertexArray->Bind();
   m_cubeTexture->Bind(0);

   m_shader->SetInt("skybox", 0);

   render::RenderCommand::DrawIndexed(m_vertexArray);

   render::RenderCommand::EnableDepthTesting();
}

} // namespace shady::scene
