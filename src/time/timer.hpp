#pragma once

#include <chrono>
#include <string>

namespace shady::time {

using timeStamp = std::chrono::time_point< std::chrono::steady_clock >;
using milliseconds = std::chrono::milliseconds;
using seconds = std::chrono::seconds;

struct TimeStep
{
   explicit TimeStep(milliseconds time);

   [[nodiscard]] std::string
   ToString() const;

   [[nodiscard]] seconds
   GetSeconds() const;

   [[nodiscard]] milliseconds
   GetMilliseconds() const;

 private:
   milliseconds time_;
};


class Timer
{
 public:
   Timer();

   [[nodiscard]] TimeStep
   ToggleTimer();

 private:
   timeStamp lastTimeStamp_;
};

} // namespace shady::time