#pragma once

#include "time/utils.hpp"
#include "trace/formatter_types.hpp"

#include <fmt/color.h>
#include <string_view>
#include <unordered_map>

#if defined(_WIN32)
#include <Windows.h>
#endif //  WIN

namespace shady::trace {

enum class TYPE
{
   TRACE,
   DEBUG,
   INFO,
   WARNING,
   FATAL
};

struct LoggerTypeHash
{
   std::size_t
   operator()(TYPE t) const
   {
      return static_cast< std::size_t >(t);
   }
};

class Logger
{
 public:
   template < typename... Args >
   static constexpr void
   Trace(std::string_view buffer, Args&&... args);

   template < typename... Args >
   static constexpr void
   Debug(std::string_view buffer, Args&&... args);

   template < typename... Args >
   static constexpr void
   Info(std::string_view buffer, Args&&... args);

   template < typename... Args >
   static constexpr void
   Warn(std::string_view buffer, Args&&... args);

   template < typename... Args >
   static constexpr void
   Fatal(std::string_view buffer, Args&&... args);

   template < TYPE LogLevel, typename... Args >
   static constexpr void
   Log(std::string_view buffer, Args&&... args);

   static void
   SetType(TYPE newType);

 private:
   static std::string
   ToString(TYPE type);

 private:
   static inline TYPE s_currentLogType = TYPE::DEBUG;

#if defined(_WIN32)
   static const inline std::unordered_map< TYPE, WORD, LoggerTypeHash > s_typeStyles = {
      {TYPE::TRACE, FOREGROUND_BLUE},
      {TYPE::DEBUG, FOREGROUND_GREEN | FOREGROUND_BLUE},
      {TYPE::INFO, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED},
      {TYPE::WARNING, FOREGROUND_GREEN | FOREGROUND_RED},
      {TYPE::FATAL, FOREGROUND_RED}};
#else
   static const inline std::unordered_map< TYPE, fmt::color, LoggerTypeHash > s_typeStyles = {
      {TYPE::TRACE, fmt::color::azure},
      {TYPE::DEBUG, fmt::color::blanched_almond},
      {TYPE::INFO, fmt::color::floral_white},
      {TYPE::WARNING, fmt::color::yellow},
      {TYPE::FATAL, fmt::color::crimson}};
#endif
};

} // namespace shady::trace

#include "logger.impl.hpp"
