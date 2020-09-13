#pragma once

#include "renderer_api.hpp"

#include <glm/glm.hpp>

namespace shady::render::opengl {

class OpenGLRendererAPI : public RendererAPI
{
 public:
   virtual ~OpenGLRendererAPI() = default;

   void
   Init() override;

   void
   SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

   void
   SetClearColor(const glm::vec4& color) override;

   void
   Clear() override;

   void
   DrawIndexed(const std::shared_ptr< VertexArray >& vertexArray, uint32_t indexCount = 0) override;
};

} // namespace shady::render::opengl
