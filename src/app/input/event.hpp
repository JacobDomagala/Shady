#pragma once

#include <stdint.h>

namespace shady::app::input {

struct Event
{
   enum class EventType
   {
      KEY,
      MOUSE_BUTTON,
      MOUSE_CURSOR,
      MOUSE_SCROLL
   };

   explicit Event(EventType t) : m_type(t)
   {
   }

   EventType m_type;
   bool m_handled = false;
};

struct KeyEvent : public Event
{
   KeyEvent(int32_t key, int32_t scanCode, int32_t action, int32_t mods)
      : Event(EventType::KEY), m_key(key), m_scanCode(scanCode), m_action(action), m_mods(mods)
   {
   }

   int32_t m_key;
   int32_t m_scanCode;
   int32_t m_action;
   int32_t m_mods;
};

struct MouseButtonEvent : public Event
{
   MouseButtonEvent(int32_t button, int32_t action, int32_t mods)
      : Event(EventType::MOUSE_BUTTON), m_buttton(button), m_action(action), m_mods(mods)
   {
   }

   int32_t m_buttton;
   int32_t m_action;
   int32_t m_mods;
};

struct CursorPositionEvent : public Event
{
   CursorPositionEvent(double x, double y, double xDelta, double yDelta)
      : Event(EventType::MOUSE_CURSOR), m_xPos(x), m_yPos(y), m_xDelta(xDelta), m_yDelta(yDelta)
   {
   }

   double m_xPos;
   double m_yPos;
   double m_xDelta;
   double m_yDelta;
};

struct MouseScrollEvent : public Event
{
   MouseScrollEvent(double xOffset, double yOffset)
      : Event(EventType::MOUSE_SCROLL), m_xOffset(xOffset), m_yOffset(yOffset)
   {
   }

   double m_xOffset;
   double m_yOffset;
};

} // namespace shady::app::input
