#pragma once

#include "time/utils.hpp"

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
   template < typename... Args >
   static constexpr void
   Trace(std::string&& buffer, const Args&... args);

   template < typename... Args >
   static constexpr void
   Debug(std::string&& buffer, const Args&... args);

   template < typename... Args >
   static constexpr void
   Info(std::string&& buffer, const Args&... args);

   template < typename... Args >
   static constexpr void
   Warn(std::string&& buffer, const Args&... args);

   template < typename... Args >
   static constexpr void
   Fatal(std::string&& buffer, const Args&... args);

   template < TYPE LogLevel, typename... Args >
   static constexpr void
   Log(std::string&& buffer, const Args&... args);

   static void
   SetType(TYPE newType);

 private:
   static std::string
   ToString(TYPE type);

 private:
   static inline TYPE s_currentLogType = TYPE::DEBUG;
};

} // namespace shady::trace

#include "logger.impl.hpp"