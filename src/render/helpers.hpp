#pragma once

#include "renderer_api.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"

#include <memory>
#include <fmt/format.h>

namespace shady::render {

template < class OpenGL, class Base, typename... Args >
std::shared_ptr< Base >
CreateSharedWrapper(const Args&... args)
{
   switch (RendererAPI::GetAPI())
   {
      case RendererAPI::API::None: {
         trace::Logger::Fatal("RendererAPI::None is currently not supported!");
         return nullptr;
      }
      break;

      case RendererAPI::API::OpenGL: {
         return std::make_shared< OpenGL >(args...);
      }
      break;
   }

   utils::Assert(false, fmt::format("API type({}) not supported!", RendererAPI::GetAPI()));
   return nullptr;
}

template < class OpenGL, class Base, typename... Args >
std::unique_ptr< Base >
CreateUniqueWrapper(const Args&... args)
{
   switch (RendererAPI::GetAPI())
   {
      case RendererAPI::API::None: {
         trace::Logger::Fatal("RendererAPI::None is currently not supported!");
         return nullptr;
      }
      break;

      case RendererAPI::API::OpenGL: {
         return std::make_unique< OpenGL >(args...);
      }
      break;
   }

   utils::Assert(false, fmt::format("API type({}) not supported!", RendererAPI::GetAPI()));
   return nullptr;
}

} // namespace shady::render