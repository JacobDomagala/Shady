#pragma once

#include "timer.hpp"

#include <string>

namespace shady::time {

class ScopedTimer
{
 public:
   ScopedTimer(const ScopedTimer&) = delete;
   ScopedTimer(ScopedTimer&&) = delete;
   ScopedTimer& operator=(const ScopedTimer&) = delete;
   ScopedTimer& operator=(ScopedTimer&&) = delete;

   explicit ScopedTimer(std::string&& logMsg);
   ~ScopedTimer();

 private:
   std::string m_logMsg;
   Timer m_timer;
};

//NOLINTNEXTLINE
#define SCOPED_TIMER(str) shady::time::ScopedTimer t(std::move(str));

} // namespace shady::time
