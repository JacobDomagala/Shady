#pragma once

#include "renderer_api.hpp"

namespace shady::render {

class Renderer
{
 public:
   static void
   Init();

   static RendererAPI::API
   GetAPI();

 private:
   static inline std::unique_ptr< RendererAPI > s_rendererAPI = RendererAPI::Create();
};

} // namespace shady::render
