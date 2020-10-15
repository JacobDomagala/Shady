#include "vertex_array.hpp"
#include "helpers.hpp"
#include "opengl/opengl_vertex_array.hpp"
#include "renderer.hpp"
#include "trace/logger.hpp"

namespace shady::render {

std::shared_ptr< VertexArray >
VertexArray::Create()
{
   return CreateSharedWrapper< opengl::OpenGLVertexArray, VertexArray >();
}

} // namespace shady::render