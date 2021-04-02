#pragma once

#include "renderer_api.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace shady::render::opengl {

class OpenGLRendererAPI : public RendererAPI
{
 public:
   virtual ~OpenGLRendererAPI() = default;

   void
   Init() override;

   void
   SetDepthFunc(DepthFunc depthFunc) override;

   void
   EnableDepthTesting() override;

   void
   DisableDepthTesting() override;

   void
   SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

   void
   SetClearColor(const glm::vec4& color) override;

   void
   Clear() override;

   void
   DrawIndexed(const std::shared_ptr< VertexArray >& vertexArray, uint32_t indexCount = 0) override;

   void
   MultiDrawElemsIndirect(uint32_t drawCount, size_t offset = 0) override;

   void
   DrawLines(uint32_t count) override;

   void
   CheckForErrors() override;

   void CreateInstance();

  private:
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;

};

} // namespace shady::render::opengl
