#include "input_manager.hpp"
#include "event.hpp"
#include "trace/logger.hpp"

#include <GLFW/glfw3.h>
#include <iostream>

namespace shady::app::input {

void
InputManager::InternalKeyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action,
                                  int32_t mods)
{
   trace::Logger::Trace("GLFW key {} {} scan code - {}", action, key, scancode);

   s_keyMap[key] = action;

   BroadcastEvent(KeyEvent{key, scancode, action, mods});
}

void
InputManager::InternalMouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action,
                                          int32_t mods)
{
   trace::Logger::Trace("GLFW mouse button {} {} {}", button, action, mods);

   BroadcastEvent(MouseButtonEvent{button, action, mods});
}

void
InputManager::InternalCursorPositionCallback(GLFWwindow* window, double x, double y)
{
   trace::Logger::Trace("GLFW cursor pos {} {}", x, y);

   auto deltaPosition = glm::dvec2(x, y) - s_mousePosition;
   s_mousePosition = glm::dvec2(x, y);

   BroadcastEvent(CursorPositionEvent{x, y, deltaPosition.x, deltaPosition.y});
}

void
InputManager::InternalMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
   trace::Logger::Trace("GLFW scroll {} {}", xoffset, yoffset);

   BroadcastEvent(MouseScrollEvent{xoffset, yoffset});
}

void
InputManager::BroadcastEvent(const Event& event)
{
   switch (event.m_type)
   {
      case Event::EventType::KEY: {
         for (auto listener : s_keyListeners)
         {
            listener->KeyCallback(static_cast< const KeyEvent& >(event));
         }
      }
      break;

      case Event::EventType::MOUSE_BUTTON: {
         for (auto listener : s_mouseButtonListeners)
         {
            listener->MouseButtonCallback(static_cast< const MouseButtonEvent& >(event));
         }
      }
      break;

      case Event::EventType::MOUSE_CURSOR: {
         for (auto listener : s_mouseMovementListeners)
         {
            listener->CursorPositionCallback(static_cast< const CursorPositionEvent& >(event));
         }
      }
      break;

      case Event::EventType::MOUSE_SCROLL: {
         for (auto listener : s_mouseScrollListeners)
         {
            listener->MouseScrollCallback(static_cast< const MouseScrollEvent& >(event));
         }
      }
      break;

      default:
         break;
   }
}

void
InputManager::Init(GLFWwindow* mainWindow)
{
   s_windowHandle = mainWindow;
   glfwGetCursorPos(s_windowHandle, &s_mousePosition.x, &s_mousePosition.y);

   glfwSetKeyCallback(s_windowHandle, InternalKeyCallback);
   glfwSetMouseButtonCallback(s_windowHandle, InternalMouseButtonCallback);
   glfwSetCursorPosCallback(s_windowHandle, InternalCursorPositionCallback);
   glfwSetScrollCallback(s_windowHandle, InternalMouseScrollCallback);

   s_keyMap.clear();
}

void
InputManager::RegisterForKeyInput(InputListener* listener)
{
   s_keyListeners.push_back(listener);
}

void
InputManager::RegisterForMouseButtonInput(InputListener* listener)
{
   s_mouseButtonListeners.push_back(listener);
}

void
InputManager::RegisterForMouseMovementInput(InputListener* listener)
{
   s_mouseMovementListeners.push_back(listener);
}

void
InputManager::RegisterForMouseScrollInput(InputListener* listener)
{
   s_mouseScrollListeners.push_back(listener);
}

void
InputManager::PollEvents()
{
   glfwPollEvents();
}

bool
InputManager::CheckKeyPressed(int32_t keyKode)
{
   return s_keyMap[keyKode];
}

glm::vec2
InputManager::GetMousePos()
{
   return s_mousePosition;
}

void
InputManager::SetMousePos(const glm::vec2& position)
{
   glfwSetCursorPos(s_windowHandle, position.x, position.y);
}

} // namespace shady::app::input