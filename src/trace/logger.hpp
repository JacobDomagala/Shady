#pragma once

//#include "Timer.hpp"

#include <fmt/format.h>
#include <string>

namespace shady::trace {

enum class TYPE
{
  TRACE,
  DEBUG,
  INFO,
  WARNING,
  FATAL
};

class Logger
{
public:
  template<typename... Args>
  static constexpr void
  Trace(std::string &&buffer, const Args &... args)
  {
    Log<TYPE::TRACE>(std::move(buffer), args...);
  }

  template<typename... Args>
  static constexpr void
  Debug(std::string &&buffer, const Args &... args)
  {
    Log<TYPE::DEBUG>(std::move(buffer), args...);
  }

  template<typename... Args>
  static constexpr void
  Info(std::string &&buffer, const Args &... args)
  {
    Log<TYPE::INFO>(std::move(buffer), args...);
  }

  template<typename... Args>
  static constexpr void
  Warn(std::string &&buffer, const Args &... args)
  {
    Log<TYPE::WARNING>(std::move(buffer), args...);
  }

  template<typename... Args>
  static constexpr void
  Fatal(std::string &&buffer, const Args &... args)
  {
    Log<TYPE::FATAL>(std::move(buffer), args...);
  }

  template<TYPE LogLevel, typename... Args>
  static constexpr void
  Log(std::string &&buffer, const Args &... args)
  {
    if (LogLevel >= s_currentLogType) {
      fmt::print("{} {}\n",
        /*Timer::GetCurrentTime(),*/ ToString(LogLevel),
        fmt::format(std::move(buffer), args...));
    }
  }

  static void
  SetType(TYPE newType)
  {
    s_currentLogType = newType;
  }

private:
  static std::string
  ToString(TYPE type)
  {
    std::string returnValue;

    if (type == TYPE::TRACE) {
      returnValue = "  [TRACE]  ";
    } else if (type == TYPE::DEBUG) {
      returnValue = "  [DEBUG]  ";
    } else if (type == TYPE::INFO) {
      returnValue = "  [INFO]   ";
    } else if (type == TYPE::WARNING) {
      returnValue = " [WARNING] ";
    } else if (type == TYPE::FATAL) {
      returnValue = "  [FATAL]  ";
    } else {
      returnValue = " [UNKNOWN] ";
    }

    return returnValue;
  }

private:
  static inline TYPE s_currentLogType = TYPE::DEBUG;
};


}// namespace shady::trace