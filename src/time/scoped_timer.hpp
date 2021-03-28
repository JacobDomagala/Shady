#pragma once

#include "timer.hpp"

#include <string>

namespace shady::time {

class ScopedTimer
{
 public:
   explicit ScopedTimer(std::string&& logMsg);
   ~ScopedTimer();

 private:
   std::string m_logMsg;
   Timer m_timer;
};

#define SCOPED_TIMER(str) shady::time::ScopedTimer t(std::move(str));

} // namespace shady::time
