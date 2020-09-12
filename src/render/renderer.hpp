#pragma once

#include "renderer_api.hpp"

#include <glad/glad.h>

namespace shady::render {

class Renderer
{
public:
  static RendererAPI::API
  GetAPI();

private:
  static inline std::unique_ptr<RendererAPI> s_rendererAPI = RendererAPI::Create();
};

}// namespace shady::render
