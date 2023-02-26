#include "timer.hpp"

#include <fmt/format.h>

namespace shady::time {

TimeStep::TimeStep(milliseconds time) : time_(time)
{
}

std::string
TimeStep::ToString() const
{
   return fmt::format("{}ms", time_.count());
}

seconds
TimeStep::GetSeconds() const
{
   return std::chrono::duration_cast< seconds >(time_);
}

milliseconds
TimeStep::GetMilliseconds() const
{
   return time_;
}


Timer::Timer() : lastTimeStamp_(std::chrono::steady_clock::now())
{
}

TimeStep
Timer::ToggleTimer()
{
   const auto timeNow = std::chrono::steady_clock::now();
   const auto step = std::chrono::duration_cast< milliseconds >(timeNow - lastTimeStamp_);

   lastTimeStamp_ = timeNow;

   return TimeStep{step};
}

} // namespace shady::time
