#include "context.hpp"

#include "opengl/opengl_context.hpp"
#include "renderer.hpp"
#include "helpers.hpp"
#include "trace/logger.hpp"

namespace shady::render {

std::unique_ptr< Context >
Context::Create(GLFWwindow* window)
{
   return CreateUniqueWrapper<opengl::OpenGLContext, Context>(window);
}

} // namespace shady::render