#include "scoped_timer.hpp"
#include "trace/logger.hpp"

namespace shady::time {

ScopedTimer::ScopedTimer(std::string&& logMsg) : m_logMsg(std::move(logMsg))
{
   static_cast<void>(m_timer.ToggleTimer());
}

//NOLINTNEXTLINE
ScopedTimer::~ScopedTimer()
{
   trace::Logger::Debug("{} took {}", m_logMsg, m_timer.ToggleTimer().ToString());
}

} // namespace shady::time