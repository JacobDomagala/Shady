#pragma once

#include "renderer.hpp"

namespace shady::render {

RendererAPI::API
Renderer::GetAPI()
{
  return s_rendererAPI->GetAPI();
}

}// namespace shady::render
