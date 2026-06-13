#include "timer.hpp"

#include <fmt/format.h>

namespace shady::time {

TimeStep::TimeStep(duration time) : time_(time)
{
}

std::string
TimeStep::ToString() const
{
   return fmt::format("{}ms", GetMilliseconds().count());
}

duration
TimeStep::GetDuration() const
{
   return time_;
}

seconds
TimeStep::GetSeconds() const
{
   return std::chrono::duration_cast< seconds >(time_);
}

milliseconds
TimeStep::GetMilliseconds() const
{
   return std::chrono::duration_cast< milliseconds >(time_);
}


Timer::Timer() : lastTimeStamp_(std::chrono::steady_clock::now())
{
}

TimeStep
Timer::ToggleTimer()
{
   const auto timeNow = std::chrono::steady_clock::now();
   const auto step = timeNow - lastTimeStamp_;

   lastTimeStamp_ = timeNow;
   elapsed_ += step;

   return TimeStep{step};
}

TimeStep
Timer::Elapsed() const
{
   return TimeStep{elapsed_};
}

} // namespace shady::time
