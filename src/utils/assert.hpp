#pragma once

#include <string_view>

namespace shady::utils {

void
Assert(bool assertion, std::string_view logMsg);

} // namespace shady::utils