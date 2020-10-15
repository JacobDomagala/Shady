
#include "renderer_api.hpp"
#include "opengl/opengl_renderer_api.hpp"
#include "helpers.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"

namespace shady::render {

std::unique_ptr< RendererAPI >
RendererAPI::Create()
{
   return CreateUniqueWrapper< opengl::OpenGLRendererAPI, RendererAPI >();
}

RendererAPI::API
RendererAPI::GetAPI()
{
   return s_API;
}

} // namespace shady::render