#include "render/renderer.hpp"
#include "render/render_command.hpp"
#include "render/renderer_3D.hpp"

namespace shady::render {

void
Renderer::Init()
{
   RenderCommand::Init();
   Renderer3D::Init();
}

RendererAPI::API
Renderer::GetAPI()
{
   return s_rendererAPI->GetAPI();
}

} // namespace shady::render
