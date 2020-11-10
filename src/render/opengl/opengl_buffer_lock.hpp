#pragma once

#include "render/buffer_lock.hpp"

#include <glad/glad.h>
#include <vector>

namespace shady::render::opengl {

struct BufferLock
{
   BufferRange m_range;
   GLsync m_syncObj;
};

class OpenGLBufferLockManager : public BufferLockManager
{
 public:
   OpenGLBufferLockManager(bool cpuRead);
   ~OpenGLBufferLockManager() override;

   void
   WaitForLockedRange(size_t _lockBeginBytes, size_t _lockLength) override;
   void
   LockRange(size_t _lockBeginBytes, size_t _lockLength) override;

 private:
   void
   Wait(GLsync* syncObj);
   void
   Cleanup(BufferLock* bufferLock);

 private:
   std::vector< BufferLock > m_bufferLocks;

   // Whether it's the CPU (true) that updates, or the GPU (false)
   bool m_cpuUpdates;
};

} // namespace shady::render::opengl