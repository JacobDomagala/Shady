#include "opengl_buffer_lock.hpp"
#include "utils/assert.hpp"
#include "trace/logger.hpp"

namespace shady::render::opengl {

GLuint64 kOneSecondInNanoSeconds = 1000000000;

OpenGLBufferLockManager::OpenGLBufferLockManager(bool cpuUpdates) : m_cpuUpdates(cpuUpdates)
{
}

OpenGLBufferLockManager::~OpenGLBufferLockManager()
{
   for (auto& lock : m_bufferLocks)
   {
      Cleanup(&lock);
   }

   m_bufferLocks.clear();
}

void
OpenGLBufferLockManager::WaitForLockedRange(size_t lockBeginBytes, size_t lockLength)
{
   BufferRange testRange = {lockBeginBytes, lockLength};
   std::vector< BufferLock > swapLocks;

   for (auto& lock : m_bufferLocks)
   {
      if (testRange.Overlaps(lock.m_range))
      {
         Wait(&lock.m_syncObj);
         Cleanup(&lock);
      }
      else
      {
         swapLocks.push_back(lock);
      }
   }

   m_bufferLocks.swap(swapLocks);
}

void
OpenGLBufferLockManager::LockRange(size_t lockBeginBytes, size_t lockLength)
{
   BufferRange newRange = {lockBeginBytes, lockLength};
   GLsync syncName = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
   BufferLock newLock = {newRange, syncName};

   m_bufferLocks.push_back(newLock);
}

void
OpenGLBufferLockManager::Wait(GLsync* syncObj)
{
   if (m_cpuUpdates)
   {
      GLbitfield waitFlags = 0;
      GLuint64 waitDuration = 0;
      while (1)
      {
         GLenum waitRet = glClientWaitSync(*syncObj, waitFlags, waitDuration);
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
      glWaitSync(*syncObj, 0, GL_TIMEOUT_IGNORED);
   }
}

void
OpenGLBufferLockManager::Cleanup(BufferLock* bufferLock)
{
   glDeleteSync(bufferLock->m_syncObj);
}

} // namespace shady::render::opengl
