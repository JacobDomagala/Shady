#include "render_command.hpp"

namespace shady::render {

void
RenderCommand::Init()
{
  s_rendererAPI->Init();
}

void
RenderCommand::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
  s_rendererAPI->SetViewport(x, y, width, height);
}

void
RenderCommand::SetClearColor(const glm::vec4 &color)
{
  s_rendererAPI->SetClearColor(color);
}

void
RenderCommand::Clear()
{
  s_rendererAPI->Clear();
}

void
RenderCommand::DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t count)
{
  s_rendererAPI->DrawIndexed(vertexArray, count);
}

}// namespace shady::render