#include "assert.hpp"
#include "trace/logger.hpp"

namespace shady::utils {

void
Assert(bool assertion, std::string&& logMsg)
{
   if (!assertion)
   {
      trace::Logger::Fatal(std::move(logMsg));
      std::terminate();
   }
}
} // namespace shady::utils