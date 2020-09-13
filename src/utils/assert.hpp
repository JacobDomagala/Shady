#pragma once

#include <string>

namespace shady::utils {

void
Assert(bool assertion, std::string&& logMsg);

} // namespace shady::utils