#pragma once

#include "logger.hpp"

#include <fmt/format.h>

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
      fmt::print("[{}]{} {}\n", time::GetTime(), ToString(LogLevel),
                 fmt::format(std::move(buffer), args...));
   }
}

} // namespace shady::trace