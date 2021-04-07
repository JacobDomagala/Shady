#pragma once

#include "shader.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <vector>

struct GLFWwindow;
namespace shady::render::vulkan {

class VulkanRenderer
{
 public:
   static void
   Initialize(GLFWwindow* windowHandle);

   static void
   Draw();

 private:
   static void
   CreateInstance();

   static void
   CreateDevice();

   static void
   CreateSwapchain(GLFWwindow* windowHandle);

   static void
   CreateImageViews();

   static void
   CreateRenderPass();

   static void
   CreateFramebuffers();

   static void
   CreateCommandPoolAndBuffers();

   static void
   CreateSyncObjects();

   static void
   CreatePipeline();

 private:
   inline static VkInstance m_instance = {};
   inline static VkDebugUtilsMessengerCreateInfoEXT m_debugCreateInfo = {};
   inline static VkDebugUtilsMessengerEXT m_debugMessenger = {};
   inline static VkSurfaceKHR m_surface = {};
   inline static VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
   inline static VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
   inline static VkDevice m_device = {};

   inline static VkQueue m_graphicsQueue = {};
   inline static VkQueue m_presentQueue = {};

   inline static VkSwapchainKHR m_swapChain = {};
   inline static std::vector< VkImage > m_swapChainImages = {};
   inline static std::vector< VkImageView > m_swapChainImageViews = {};
   inline static std::vector< VkFramebuffer > m_swapChainFramebuffers = {};
   inline static VkFormat m_swapChainImageFormat = {};
   inline static VkExtent2D m_swapChainExtent = {};

   inline static VkRenderPass m_renderPass = {};
   inline static VkPipelineLayout m_pipelineLayout = {};
   inline static VkPipeline m_graphicsPipeline = {};

   inline static VkCommandPool m_commandPool = {};
   inline static std::vector< VkCommandBuffer > m_commandBuffers = {};

   inline static std::vector< VkSemaphore > m_imageAvailableSemaphores = {};
   inline static std::vector< VkSemaphore > m_renderFinishedSemaphores = {};
   inline static std::vector< VkFence > m_inFlightFences = {};
   inline static std::vector< VkFence > m_imagesInFlight = {};

   inline static VkBuffer m_vertexBuffer = {};
   inline static VkDeviceMemory m_vertexBufferMemory = {};
};

} // namespace shady::render::vulkan
