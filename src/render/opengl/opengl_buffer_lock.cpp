#include "opengl_buffer_lock.hpp"
#include "utils/assert.hpp"

namespace shady::render::opengl {

GLuint64 kOneSecondInNanoSeconds = 1000000000;

OpenGLBufferLockManager::OpenGLBufferLockManager(bool _cpuUpdates) : mCPUUpdates(_cpuUpdates)
{
}

OpenGLBufferLockManager::~OpenGLBufferLockManager()
{
   for (auto& lock : mBufferLocks)
   {
      cleanup(&lock);
   }

   mBufferLocks.clear();
}

void
OpenGLBufferLockManager::WaitForLockedRange(size_t _lockBeginBytes, size_t _lockLength)
{
   BufferRange testRange = {_lockBeginBytes, _lockLength};
   std::vector< BufferLock > swapLocks;

   for (auto& lock : mBufferLocks)
   {
      if (testRange.Overlaps(lock.mRange))
      {
         wait(&lock.mSyncObj);
         cleanup(&lock);
      }
      else
      {
         swapLocks.push_back(lock);
      }
   }

   mBufferLocks.swap(swapLocks);
}

void
OpenGLBufferLockManager::LockRange(size_t _lockBeginBytes, size_t _lockLength)
{
   BufferRange newRange = {_lockBeginBytes, _lockLength};
   GLsync syncName = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
   BufferLock newLock = {newRange, syncName};

   mBufferLocks.push_back(newLock);
}

void
OpenGLBufferLockManager::wait(GLsync* _syncObj)
{
   if (mCPUUpdates)
   {
      GLbitfield waitFlags = 0;
      GLuint64 waitDuration = 0;
      while (1)
      {
         GLenum waitRet = glClientWaitSync(*_syncObj, waitFlags, waitDuration);
         if (waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED)
         {
            return;
         }

         utils::Assert(waitRet != GL_WAIT_FAILED, "glClientWaitSync failure!");

         // After the first time, need to start flushing, and wait for a looong time.
         waitFlags = GL_SYNC_FLUSH_COMMANDS_BIT;
         waitDuration = kOneSecondInNanoSeconds;
      }
   }
   else
   {
      glWaitSync(*_syncObj, 0, GL_TIMEOUT_IGNORED);
   }
}

void
OpenGLBufferLockManager::cleanup(BufferLock* _bufferLock)
{
   glDeleteSync(_bufferLock->mSyncObj);
}

} // namespace shady::render::opengl