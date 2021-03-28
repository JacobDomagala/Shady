#pragma once

#include "logger.hpp"

namespace shady::trace {


template < typename... Args >
constexpr void
Logger::Trace(std::string&& buffer, const Args&... args)
{
   Log< TYPE::TRACE >(std::move(buffer), args...);
}

template < typename... Args >
constexpr void
Logger::Debug(std::string&& buffer, const Args&... args)
{
   Log< TYPE::DEBUG >(std::move(buffer), args...);
}

template < typename... Args >
constexpr void
Logger::Info(std::string&& buffer, const Args&... args)
{
   Log< TYPE::INFO >(std::move(buffer), args...);
}

template < typename... Args >
constexpr void
Logger::Warn(std::string&& buffer, const Args&... args)
{
   Log< TYPE::WARNING >(std::move(buffer), args...);
}

template < typename... Args >
constexpr void
Logger::Fatal(std::string&& buffer, const Args&... args)
{
   Log< TYPE::FATAL >(std::move(buffer), args...);
}

template < TYPE LogLevel, typename... Args >
constexpr void
Logger::Log(std::string&& buffer, const Args&... args)
{
   if (LogLevel >= s_currentLogType)
   {
#if defined(_WIN32)
      auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      SetConsoleTextAttribute(hConsole, s_typeStyles.at(LogLevel));

      fmt::print("[{}]{} {}\n", time::GetTime(), ToString(LogLevel),
                 fmt::format(std::move(buffer), args...));

      // Set the color to white
      SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
#else
      fmt::print(fmt::fg(s_typeStyles.at(LogLevel)), "[{}]{} {}\n", time::GetTime(),
                 ToString(LogLevel), fmt::format(std::move(buffer), args...));
#endif
   }
}

} // namespace shady::trace