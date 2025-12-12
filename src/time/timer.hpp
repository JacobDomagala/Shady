#pragma once

#include <chrono>
#include <string>

namespace shady::time {

using timeStamp = std::chrono::time_point< std::chrono::steady_clock >;
using duration = std::chrono::steady_clock::duration;
using milliseconds = std::chrono::milliseconds;
using seconds = std::chrono::seconds;

struct TimeStep
{
   explicit TimeStep(duration time);

   [[nodiscard]] std::string
   ToString() const;

   [[nodiscard]] duration
   GetDuration() const;

   [[nodiscard]] seconds
   GetSeconds() const;

   [[nodiscard]] milliseconds
   GetMilliseconds() const;

 private:
   duration time_;
};


class Timer
{
 public:
   Timer();

   [[nodiscard]] TimeStep
   ToggleTimer();

   [[nodiscard]] TimeStep
   Elapsed() const;

 private:
   timeStamp lastTimeStamp_;
   duration elapsed_ = {};
};

} // namespace shady::time
