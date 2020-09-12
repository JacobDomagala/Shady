#include "timer.hpp"

#include <fmt/format.h>

namespace shady::time {

TimeStep::TimeStep(milliseconds time) : m_time(time)
{
}

std::string
TimeStep::ToString() const
{
   return fmt::format("{}ms", m_time.count());
}

seconds
TimeStep::GetSeconds() const
{
   return std::chrono::duration_cast< seconds >(m_time);
}

milliseconds
TimeStep::GetMilliseconds() const
{
   return m_time;
}


Timer::Timer()
{
   m_lastTimeStamp = std::chrono::steady_clock::now();
}

TimeStep
Timer::ToggleTimer()
{
   const auto timeNow = std::chrono::steady_clock::now();
   const auto step = std::chrono::duration_cast< milliseconds >(timeNow - m_lastTimeStamp);

   m_lastTimeStamp = timeNow;

   return TimeStep{step};
}

} // namespace shady::time