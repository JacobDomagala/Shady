
#include "opengl_renderer_api.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
// #include <glad/glad.h>
#include <optional>
#include <set>

#undef max
#undef min
namespace shady::render::opengl {

struct QueueFamilyIndices
{
   std::optional< uint32_t > graphicsFamily;
   std::optional< uint32_t > presentFamily;

   bool
   isComplete()
   {
      return graphicsFamily.has_value() && presentFamily.has_value();
   }
};

struct SwapChainSupportDetails
{
   VkSurfaceCapabilitiesKHR capabilities;
   std::vector< VkSurfaceFormatKHR > formats;
   std::vector< VkPresentModeKHR > presentModes;
};

const std::vector< const char* > deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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

QueueFamilyIndices
findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
   QueueFamilyIndices indices;

   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

   std::vector< VkQueueFamilyProperties > queueFamilies(queueFamilyCount);
   vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

   int i = 0;
   for (const auto& queueFamily : queueFamilies)
   {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
         indices.graphicsFamily = i;
      }

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

      if (presentSupport)
      {
         indices.presentFamily = i;
      }

      if (indices.isComplete())
      {
         break;
      }

      i++;
   }

   return indices;
}

SwapChainSupportDetails
querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
   SwapChainSupportDetails details;

   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

   uint32_t formatCount;
   vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

   if (formatCount != 0)
   {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
   }

   uint32_t presentModeCount;
   vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

   if (presentModeCount != 0)
   {
      details.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                                details.presentModes.data());
   }

   return details;
}

bool
checkDeviceExtensionSupport(VkPhysicalDevice device)
{
   uint32_t extensionCount;
   vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

   std::vector< VkExtensionProperties > availableExtensions(extensionCount);
   vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                        availableExtensions.data());

   std::set< std::string > requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

   for (const auto& extension : availableExtensions)
   {
      requiredExtensions.erase(extension.extensionName);
   }

   return requiredExtensions.empty();
}

bool
isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
   QueueFamilyIndices indices = findQueueFamilies(device, surface);

   bool extensionsSupported = checkDeviceExtensionSupport(device);

   bool swapChainAdequate = false;
   if (extensionsSupported)
   {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
      swapChainAdequate =
         !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
   }

   VkPhysicalDeviceFeatures supportedFeatures;
   vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

   return indices.isComplete() && extensionsSupported && swapChainAdequate
          && supportedFeatures.samplerAnisotropy;
}

VkSampleCountFlagBits
getMaxUsableSampleCount(VkPhysicalDevice& physicalDevice)
{
   VkPhysicalDeviceProperties physicalDeviceProperties;
   vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

   VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts
                               & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
   if (counts & VK_SAMPLE_COUNT_64_BIT)
   {
      return VK_SAMPLE_COUNT_64_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_32_BIT)
   {
      return VK_SAMPLE_COUNT_32_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_16_BIT)
   {
      return VK_SAMPLE_COUNT_16_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_8_BIT)
   {
      return VK_SAMPLE_COUNT_8_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_4_BIT)
   {
      return VK_SAMPLE_COUNT_4_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_2_BIT)
   {
      return VK_SAMPLE_COUNT_2_BIT;
   }

   return VK_SAMPLE_COUNT_1_BIT;
}

VkSurfaceFormatKHR
chooseSwapSurfaceFormat(const std::vector< VkSurfaceFormatKHR >& availableFormats)
{
   for (const auto& availableFormat : availableFormats)
   {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
          && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      {
         return availableFormat;
      }
   }

   return availableFormats[0];
}

VkPresentModeKHR
chooseSwapPresentMode(const std::vector< VkPresentModeKHR >& availablePresentModes)
{
   for (const auto& availablePresentMode : availablePresentModes)
   {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      {
         return availablePresentMode;
      }
   }

   return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* windowHandle)
{
   if (capabilities.currentExtent.width != UINT32_MAX)
   {
      return capabilities.currentExtent;
   }
   else
   {
      int width, height;
      glfwGetFramebufferSize(windowHandle, &width, &height);

      VkExtent2D actualExtent = {static_cast< uint32_t >(width), static_cast< uint32_t >(height)};

      actualExtent.width =
         ::glm::max(capabilities.minImageExtent.width,
                    ::glm::min(capabilities.maxImageExtent.width, actualExtent.width));
      actualExtent.height =
         ::glm::max(capabilities.minImageExtent.height,
                    ::glm::min(capabilities.maxImageExtent.height, actualExtent.height));

      return actualExtent;
   }
}

void
OpenGLRendererAPI::InitializeVulkan(GLFWwindow* windowHandle)
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

   if (glfwCreateWindowSurface(m_instance, windowHandle, nullptr, &m_surface) != VK_SUCCESS)
   {
      throw std::runtime_error("failed to create window surface!");
   }

   uint32_t deviceCount = 0;
   vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

   if (deviceCount == 0)
   {
      throw std::runtime_error("failed to find GPUs with Vulkan support!");
   }

   std::vector< VkPhysicalDevice > devices(deviceCount);
   vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

   for (const auto& device : devices)
   {
      if (isDeviceSuitable(device, m_surface))
      {
         m_physicalDevice = device;
         m_msaaSamples = getMaxUsableSampleCount(m_physicalDevice);
         break;
      }
   }

   if (m_physicalDevice == VK_NULL_HANDLE)
   {
      throw std::runtime_error("failed to find a suitable GPU!");
   }

   QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);

   std::vector< VkDeviceQueueCreateInfo > queueCreateInfos;
   std::set< uint32_t > uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                               indices.presentFamily.value()};

   float queuePriority = 1.0f;
   for (uint32_t queueFamily : uniqueQueueFamilies)
   {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
   }

   VkPhysicalDeviceFeatures deviceFeatures{};
   deviceFeatures.samplerAnisotropy = VK_TRUE;

   VkDeviceCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

   createInfo.queueCreateInfoCount = static_cast< uint32_t >(queueCreateInfos.size());
   createInfo.pQueueCreateInfos = queueCreateInfos.data();

   createInfo.pEnabledFeatures = &deviceFeatures;

   createInfo.enabledExtensionCount = static_cast< uint32_t >(deviceExtensions.size());
   createInfo.ppEnabledExtensionNames = deviceExtensions.data();

   if (enableValidationLayers)
   {
      createInfo.enabledLayerCount = static_cast< uint32_t >(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
   }
   else
   {
      createInfo.enabledLayerCount = 0;
   }

   if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
   {
      throw std::runtime_error("failed to create logical device!");
   }

   vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
   vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);

   SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice, m_surface);

   VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
   VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
   VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, windowHandle);

   uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
   if (swapChainSupport.capabilities.maxImageCount > 0
       && imageCount > swapChainSupport.capabilities.maxImageCount)
   {
      imageCount = swapChainSupport.capabilities.maxImageCount;
   }

   /*
   *     CREATE SWAP CHAINS
   */
   VkSwapchainCreateInfoKHR swapChainCreateInfo{};
   swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   swapChainCreateInfo.surface = m_surface;

   swapChainCreateInfo.minImageCount = imageCount;
   swapChainCreateInfo.imageFormat = surfaceFormat.format;
   swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
   swapChainCreateInfo.imageExtent = extent;
   swapChainCreateInfo.imageArrayLayers = 1;
   swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

   QueueFamilyIndices indicesSecond = findQueueFamilies(m_physicalDevice, m_surface);
   uint32_t queueFamilyIndices[2] = {indicesSecond.graphicsFamily.value(),
                                     indicesSecond.presentFamily.value()};

   if (indicesSecond.graphicsFamily != indicesSecond.presentFamily)
   {
      swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      swapChainCreateInfo.queueFamilyIndexCount = 2;
      swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
   }
   else
   {
      swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
   }

   swapChainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
   swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
   swapChainCreateInfo.presentMode = presentMode;
   swapChainCreateInfo.clipped = VK_TRUE;

   if (vkCreateSwapchainKHR(m_device, &swapChainCreateInfo, nullptr, &m_swapChain) != VK_SUCCESS)
   {
      throw std::runtime_error("failed to create swap chain!");
   }

   vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
   m_swapChainImages.resize(imageCount);
   vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

   m_swapChainImageFormat = surfaceFormat.format;
   m_swapChainExtent = extent;


   /*
   *     CREATE IMAGE VIEWS
   */
   m_swapChainImageViews.resize(m_swapChainImages.size());

   for (size_t i = 0; i < m_swapChainImages.size(); i++)
   {
      VkImageViewCreateInfo imageViewCreateInfo{};
      imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      imageViewCreateInfo.image = m_swapChainImages[i];
      imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      imageViewCreateInfo.format = m_swapChainImageFormat;
      imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
      imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
      imageViewCreateInfo.subresourceRange.levelCount = 1;
      imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
      imageViewCreateInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS)
      {
         throw std::runtime_error("failed to create image views!");
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
   appInfo.pApplicationName = "Shady";
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

   if constexpr (enableValidationLayers)
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
OpenGLRendererAPI::Init()
{
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
