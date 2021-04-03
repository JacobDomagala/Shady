#pragma once

#include "logger.hpp"

namespace shady::trace {


template < typename... Args >
constexpr void
Logger::Trace(std::string_view buffer, Args&&... args)
{
   Log< TYPE::TRACE >(std::move(buffer), std::forward< Args >(args)...);
}

template < typename... Args >
constexpr void
Logger::Debug(std::string_view buffer, Args&&... args)
{
   Log< TYPE::DEBUG >(std::move(buffer), std::forward< Args >(args)...);
}

template < typename... Args >
constexpr void
Logger::Info(std::string_view buffer, Args&&... args)
{
   Log< TYPE::INFO >(std::move(buffer), std::forward< Args >(args)...);
}

template < typename... Args >
constexpr void
Logger::Warn(std::string_view buffer, Args&&... args)
{
   Log< TYPE::WARNING >(std::move(buffer), std::forward< Args >(args)...);
}

template < typename... Args >
constexpr void
Logger::Fatal(std::string_view buffer, Args&&... args)
{
   Log< TYPE::FATAL >(std::move(buffer), std::forward< Args >(args)...);
}

template < TYPE LogLevel, typename... Args >
constexpr void
Logger::Log(std::string_view buffer, Args&&... args)
{
   if (LogLevel >= s_currentLogType)
   {
      if constexpr (_WIN32)
      {
         auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
         SetConsoleTextAttribute(hConsole, s_typeStyles.at(LogLevel));

         fmt::print("[{}]{} {}\n", time::GetTime(), ToString(LogLevel),
                    fmt::format(buffer, std::forward< Args >(args)...));

         // Set the color to white
         SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
      }
      else
      {
         fmt::print(fmt::fg(s_typeStyles.at(LogLevel)), "[{}]{} {}\n", time::GetTime(),
                    ToString(LogLevel),
                    fmt::format(buffer, std::forward< Args >(args)...));
      }
   }
}

} // namespace shady::trace
