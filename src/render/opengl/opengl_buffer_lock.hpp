#pragma once

#include "render/buffer_lock.hpp"

#include <glad/glad.h>
#include <vector>

namespace shady::render::opengl {

struct BufferLock
{
   BufferRange mRange;
   GLsync mSyncObj;
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
   wait(GLsync* _syncObj);
   void
   cleanup(BufferLock* _bufferLock);

   std::vector< BufferLock > mBufferLocks;

   // Whether it's the CPU (true) that updates, or the GPU (false)
   bool mCPUUpdates;
};

} // namespace shady::render::opengl