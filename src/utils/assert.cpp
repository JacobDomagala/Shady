#include "assert.hpp"
#include "trace/logger.hpp"

namespace shady::utils {

void
Assert(bool assertion, std::string_view logMsg)
{
   if (!assertion)
   {
      trace::Logger::Fatal("{}", logMsg);
      std::terminate();
   }
}
} // namespace shady::utils