#pragma once

#include "event.hpp"

namespace shady::app::input {

class InputListener
{
 public:
   InputListener() = default;
   InputListener(const InputListener&) = default;
   InputListener(InputListener&&) = default;
   InputListener& operator=(const InputListener&) = default;
   InputListener& operator=(InputListener&&) = default;
   virtual ~InputListener() = default;

   virtual void
   KeyCallback(const KeyEvent& /*event*/)
   {
   }

   virtual void
   MouseButtonCallback(const MouseButtonEvent& /*event*/)
   {
   }

   virtual void
   CursorPositionCallback(const CursorPositionEvent& /*event*/)
   {
   }

   virtual void
   MouseScrollCallback(const MouseScrollEvent& /*event*/)
   {
   }
};

} // namespace shady::app::input
