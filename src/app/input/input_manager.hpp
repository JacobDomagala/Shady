#pragma once

#include "input_listener.hpp"

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

struct GLFWwindow;

namespace shady::app::input {

struct Event;

class InputManager
{
 public:
   // Functions used by InputListener to register for input callbacks
   static void
   RegisterForKeyInput(InputListener* listener);

   static void
   RegisterForMouseButtonInput(InputListener* listener);

   static void
   RegisterForMouseMovementInput(InputListener* listener);

   static void
   RegisterForMouseScrollInput(InputListener* listener);

   // @brief Polling function to check if key is currenty pressed
   // @param keyKode Key in which user is interested
   // @return True if key is pressed, False otherwise
   static bool
   CheckKeyPressed(int32_t keyKode);

   // @brief Polling function to check if mouse button is currenty pressed
   // @param button is this button pressed
   // @return True if button is pressed, False otherwise
   static bool
   CheckButtonPressed(int32_t button);

   // @brief Polling function to get current mouse position
   // @return Mouse position relative to window
   static glm::vec2
   GetMousePos();

   static void
   SetMousePos(const glm::vec2& position);

   // @brief Called each frame to update the input
   static void
   PollEvents();

   // @brief Initialize function which should be called once at the start of the program
   // @param mainWindow GLFW window handle for which input will be handled
   static void
   Init(GLFWwindow* mainWindow);

   // callbacks for GLFW input
   static void
   InternalKeyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action,
                       int32_t mods);
   static void
   InternalMouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods);
   static void
   InternalCursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
   static void
   InternalMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

 private:
   static void
   BroadcastEvent(const Event& event);

 private:
   static inline std::vector< InputListener* > s_keyListeners = {};
   static inline std::vector< InputListener* > s_mouseButtonListeners = {};
   static inline std::vector< InputListener* > s_mouseMovementListeners = {};
   static inline std::vector< InputListener* > s_mouseScrollListeners = {};

   // in future handle input from multiple windows?
   static inline GLFWwindow* s_windowHandle = nullptr;

   static inline glm::dvec2 s_mousePosition = {};
   static inline std::unordered_map< int32_t, bool > s_mouseButtonMap = {};
   static inline std::unordered_map< int32_t, bool > s_keyMap = {};
};

} // namespace shady::app::input
