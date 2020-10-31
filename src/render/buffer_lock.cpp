#include "render/buffer_lock.hpp"
#include "helpers.hpp"
#include "render/opengl/opengl_buffer_lock.hpp"

namespace shady::render {

std::shared_ptr< BufferLockManager >
BufferLockManager::Create(bool cpuRead)
{
   return CreateSharedWrapper< opengl::OpenGLBufferLockManager, BufferLockManager >(cpuRead);
}

} // namespace shady::render