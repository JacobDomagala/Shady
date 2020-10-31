#pragma once

#include <memory>

namespace shady::render {

struct BufferRange
{
   size_t mStartOffset;
   size_t mLength;

   bool
   Overlaps(const BufferRange& _rhs) const
   {
      return mStartOffset < (_rhs.mStartOffset + _rhs.mLength)
             && _rhs.mStartOffset < (mStartOffset + mLength);
   }
};

class BufferLockManager
{
 public:
   virtual ~BufferLockManager() = default;

   virtual void
   WaitForLockedRange(size_t _lockBeginBytes, size_t _lockLength) = 0;

   virtual void
   LockRange(size_t _lockBeginBytes, size_t _lockLength) = 0;

   static std::shared_ptr< BufferLockManager >
   Create(bool cpuRead);
};

} // namespace shady::render