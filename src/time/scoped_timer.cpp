#include "scoped_timer.hpp"
#include "trace/logger.hpp"

namespace shady::time {

ScopedTimer::ScopedTimer(std::string&& logMsg) : m_logMsg(std::move(logMsg))
{
   m_timer.ToggleTimer();
}

ScopedTimer::~ScopedTimer()
{
   trace::Logger::Info("{} took {}", m_logMsg, m_timer.ToggleTimer().ToString());
}

} // namespace shady::time