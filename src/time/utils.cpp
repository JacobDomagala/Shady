#pragma once

#include "utils.hpp"

#include <fmt/chrono.h>

namespace shady::time {

std::string
GetTime()
{
  auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  return fmt::format("{:%H:%M:%S}", fmt::localtime(time));
}

}// namespace shady::time
