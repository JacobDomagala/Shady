#pragma once

#include <memory>

struct GLFWwindow;

namespace shady::render {

class Context
{
 public:
   virtual ~Context() = default;

   virtual void
   Init() = 0;

   virtual void
   SwapBuffers() = 0;

   static std::unique_ptr< Context >
   Create(GLFWwindow* window);
};

} // namespace shady::render