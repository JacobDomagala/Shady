#pragma once

#include <cstdint>

namespace shady::app::input {

struct Event
{
   Event(const Event&) = default;
   Event(Event&&) = default;
   Event& operator=(const Event&) = default;
   Event& operator=(Event&&) = default;
   virtual ~Event() = default;

   enum class EventType
   {
      KEY,
      MOUSE_BUTTON,
      MOUSE_CURSOR,
      MOUSE_SCROLL
   };

   explicit Event(EventType t) : type_(t)
   {
   }

   [[nodiscard]] EventType
   GetType() const
   {
      return type_;
   }

   [[nodiscard]] bool
   IsHandled() const
   {
      return handled_;
   }

 private:
   EventType type_;
   bool handled_ = false;
};

struct KeyEvent : public Event
{
   KeyEvent(int32_t key, int32_t scanCode, int32_t action, int32_t mods)
      : Event(EventType::KEY), key_(key), scanCode_(scanCode), action_(action), mods_(mods)
   {
   }

   [[nodiscard]] int32_t
   GetKey() const
   {
      return key_;
   }

   [[nodiscard]] int32_t
   GetScanCode() const
   {
      return scanCode_;
   }

   [[nodiscard]] int32_t
   GetAction() const
   {
      return action_;
   }

   [[nodiscard]] int32_t
   GetMods() const
   {
      return mods_;
   }

 private:
   int32_t key_;
   int32_t scanCode_;
   int32_t action_;
   int32_t mods_;
};

struct MouseButtonEvent : public Event
{
   MouseButtonEvent(int32_t button, int32_t action, int32_t mods)
      : Event(EventType::MOUSE_BUTTON), buttton_(button), action_(action), mods_(mods)
   {
   }

   [[nodiscard]] int32_t
   GetButton() const
   {
      return buttton_;
   }

   [[nodiscard]] int32_t
   GetAction() const
   {
      return action_;
   }

   [[nodiscard]] int32_t
   GetMods() const
   {
      return mods_;
   }

 private:
   int32_t buttton_;
   int32_t action_;
   int32_t mods_;
};

struct CursorPositionEvent : public Event
{
   CursorPositionEvent(double x, double y, double xDelta, double yDelta)
      : Event(EventType::MOUSE_CURSOR), xPos_(x), yPos_(y), xDelta_(xDelta), yDelta_(yDelta)
   {
   }

   [[nodiscard]] double
   GetXPos() const
   {
      return xPos_;
   }

   [[nodiscard]] double
   GetYPos() const
   {
      return yPos_;
   }

   [[nodiscard]] double
   GetXDelta() const
   {
      return xDelta_;
   }

   [[nodiscard]] double
   GetYDelta() const
   {
      return yDelta_;
   }

 private:
   double xPos_;
   double yPos_;
   double xDelta_;
   double yDelta_;
};

struct MouseScrollEvent : public Event
{
   MouseScrollEvent(double xOffset, double yOffset)
      : Event(EventType::MOUSE_SCROLL), xOffset_(xOffset), yOffset_(yOffset)
   {
   }

   [[nodiscard]] double
   GetXOffset() const
   {
      return xOffset_;
   }

   [[nodiscard]] double
   GetYOffset() const
   {
      return yOffset_;
   }

 private:
   double xOffset_;
   double yOffset_;
};

} // namespace shady::app::input
