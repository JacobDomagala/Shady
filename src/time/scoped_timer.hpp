#pragma once

#include "timer.hpp"

#include <string>

namespace shady::time {

class ScopedTimer
{
 public:
   ScopedTimer(std::string&& logMsg);
   ~ScopedTimer();

 private:
   std::string m_logMsg;
   Timer m_timer;
};

} // namespace shady::time