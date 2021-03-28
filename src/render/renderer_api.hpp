#pragma once

#include "vertex_array.hpp"

#include <glm/glm.hpp>

namespace shady::render {

enum class DepthFunc
{
   LEQUAL,
   LESS
};

class RendererAPI
{
 public:
   enum class API
   {
      None = 0,
      OpenGL = 1
   };

 public:
   virtual ~RendererAPI() = default;

   virtual void
   Init() = 0;

   virtual void
   SetDepthFunc(DepthFunc depthFunc) = 0;

   virtual void
   EnableDepthTesting() = 0;

   virtual void
   DisableDepthTesting() = 0;

   virtual void
   SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

   virtual void
   SetClearColor(const glm::vec4& color) = 0;

   virtual void
   Clear() = 0;

   virtual void
   DrawIndexed(const std::shared_ptr< VertexArray >& vertexArray, uint32_t indexCount = 0) = 0;

   virtual void
   MultiDrawElemsIndirect(uint32_t drawCount, size_t offset = 0) = 0;

   virtual void
   DrawLines(uint32_t count) = 0;

   virtual void
   CheckForErrors() = 0;

   static API
   GetAPI();

   static std::unique_ptr< RendererAPI >
   Create();

 private:
   static inline API s_API = API::OpenGL;
};

} // namespace shady::render
