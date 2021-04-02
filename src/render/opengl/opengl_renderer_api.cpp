
#include "opengl_renderer_api.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"

#include <GLFW/glfw3.h>
// #include <glad/glad.h>


namespace shady::render::opengl {

VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
static constexpr bool enableValidationLayers = true;
const std::vector< const char* > validationLayers = {"VK_LAYER_KHRONOS_validation"};

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
   trace::Logger::Debug("validation layer: {}", pCallbackData->pMessage);

   return VK_FALSE;
}

std::vector< const char* >
getRequiredExtensions()
{
   uint32_t glfwExtensionCount = 0;
   const char** glfwExtensions;
   glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

   std::vector< const char* > extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

   if (enableValidationLayers)
   {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
   }

   return extensions;
}

bool
checkValidationLayerSupport()
{
   uint32_t layerCount;
   vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

   std::vector< VkLayerProperties > availableLayers(layerCount);
   vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

   for (const char* layerName : validationLayers)
   {
      bool layerFound = false;

      for (const auto& layerProperties : availableLayers)
      {
         if (strcmp(layerName, layerProperties.layerName) == 0)
         {
            layerFound = true;
            break;
         }
      }

      if (!layerFound)
      {
         return false;
      }
   }

   return true;
}
void
populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
   createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
   createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
   createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
   createInfo.pfnUserCallback = debugCallback;
}

VkResult
CreateDebugUtilsMessengerEXT(VkInstance instance,
                             const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                             const VkAllocationCallbacks* pAllocator,
                             VkDebugUtilsMessengerEXT* pDebugMessenger)
{
   auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
   if (func != nullptr)
   {
      return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
   }
   else
   {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
   }
}

void
OpenGLRendererAPI::Init()
{
   CreateInstance();

   if constexpr (enableValidationLayers)
   {
      VkDebugUtilsMessengerCreateInfoEXT createInfo;
      populateDebugMessengerCreateInfo(createInfo);

      if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger)
          != VK_SUCCESS)
      {
         throw std::runtime_error("failed to set up debug messenger!");
      }
   }
}

void
OpenGLRendererAPI::CreateInstance()
{
 if (enableValidationLayers && !checkValidationLayerSupport())
   {
      throw std::runtime_error("validation layers requested, but not available!");
   }

   VkApplicationInfo appInfo{};
   appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   appInfo.pApplicationName = "Hello Triangle";
   appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
   appInfo.pEngineName = "No Engine";
   appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
   appInfo.apiVersion = VK_API_VERSION_1_0;

   VkInstanceCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   createInfo.pApplicationInfo = &appInfo;

   auto extensions = getRequiredExtensions();
   createInfo.enabledExtensionCount = static_cast< uint32_t >(extensions.size());
   createInfo.ppEnabledExtensionNames = extensions.data();

   if constexpr(enableValidationLayers)
   {
      createInfo.enabledLayerCount = static_cast< uint32_t >(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();

      populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (void*)&debugCreateInfo;
   }
   else
   {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
   }

   if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
   {
      throw std::runtime_error("failed to create instance!");
   }
}

void
OpenGLRendererAPI::SetDepthFunc(DepthFunc depthFunc)
{
   // glDepthFunc(depthFunc == DepthFunc::LEQUAL ? GL_LEQUAL : GL_LESS);
}

void
OpenGLRendererAPI::EnableDepthTesting()
{
   // glEnable(GL_DEPTH_TEST);
}

void
OpenGLRendererAPI::DisableDepthTesting()
{
   // glDisable(GL_DEPTH_TEST);
}

void
OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
   // glViewport(x, y, width, height);
}

void
OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
{
   // glClearColor(color.r, color.g, color.b, color.a);
}

void
OpenGLRendererAPI::Clear()
{
   // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
OpenGLRendererAPI::DrawIndexed(const std::shared_ptr< VertexArray >& vertexArray,
                               uint32_t indexCount)
{
   // auto count =
   //    static_cast< GLsizei >(indexCount ? indexCount :
   //    vertexArray->GetIndexBuffer()->GetCount());
   // glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}

void
OpenGLRendererAPI::MultiDrawElemsIndirect(uint32_t drawCount, size_t offset)
{
   // glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
   // glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, reinterpret_cast< void* >(offset),
   //                             drawCount, 0);
}

void
OpenGLRendererAPI::DrawLines(uint32_t count)
{
   // glDrawArrays(GL_LINES, 0, count * 2);
}

void
OpenGLRendererAPI::CheckForErrors()
{
   // const auto value = glGetError();

   // switch (value)
   // {
   //    case GL_INVALID_FRAMEBUFFER_OPERATION: {
   //       utils::Assert(false, "Framebuffer error!");
   //    }
   //    break;

   //    case GL_NONE: {
   //       // No error
   //    }
   //    break;

   //    default: {
   //       trace::Logger::Fatal("Unspecified error {:#04x} !", value);
   //    }
   // }
}

} // namespace shady::render::opengl
