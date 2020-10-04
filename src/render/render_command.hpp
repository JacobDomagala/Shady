#pragma once

#include "renderer_api.hpp"

namespace shady::render {

class RenderCommand
{
 public:
   static void
   Init();

   static void
   SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

   static void
   SetClearColor(const glm::vec4& color);

   static void
   Clear();

   static void
   DrawIndexed(const std::shared_ptr< VertexArray >& vertexArray, size_t count = 0);

   static void
   DrawLines(uint32_t count);

 private:
   static inline std::unique_ptr< RendererAPI > s_rendererAPI = RendererAPI::Create();
};

} // namespace shady::render
