#pragma once

#include "renderer.hpp"
#include "render_command.hpp"

namespace shady::render {
void
Renderer::Init()
{
   RenderCommand::Init();  
}

RendererAPI::API
Renderer::GetAPI()
{
   return s_rendererAPI->GetAPI();
}

} // namespace shady::render
