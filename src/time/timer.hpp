#pragma once

#include <chrono>
#include <string>

namespace shady::time {

using timeStamp = std::chrono::time_point<std::chrono::steady_clock>;
using milliseconds = std::chrono::milliseconds;
using seconds = std::chrono::seconds;

struct TimeStep
{
  explicit TimeStep(milliseconds time);

  std::string
  ToString() const;

  seconds
  GetSeconds() const;

  milliseconds
  GetMilliseconds() const;

private:
  milliseconds m_time;
};


class Timer
{
public:
  Timer();

  TimeStep
  ToggleTimer();

private:
  timeStamp m_lastTimeStamp;
};

}// namespace shady::time