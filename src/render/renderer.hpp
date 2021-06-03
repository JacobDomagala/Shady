#pragma once

#include "deferred_pipeline.hpp"
#include "vertex.hpp"
#include "types.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace shady::render {

class Renderer
{
 public:
   static void
   Initialize(GLFWwindow* windowHandle);

   static void
   CreateRenderPipeline();

   static void
   Draw();

   static void
   MeshLoaded(const std::vector< Vertex >& vertices,
              const std::vector< uint32_t >& indicies, const TextureMaps& textures,
              const glm::mat4& modelMat);

   static void
   CreateCommandBufferForDeferred();

   static void
   UpdateUniformBuffer(const scene::Camera* camera, const scene::Light* light);

 private:
   static void
   SetupData();

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
   CreateCommandPool();

   static void
   CreateFramebuffers();

   static void
   CreateSyncObjects();

   static void
   CreatePipelineCache();

   static void
   CreateVertexBuffer();

   static void
   CreateIndexBuffer();

   static void
   CreateUniformBuffers();

   static void
   CreateDepthResources();

   static void
   CreateColorResources();

 private:
   inline static VkDebugUtilsMessengerCreateInfoEXT m_debugCreateInfo = {};
   inline static VkDebugUtilsMessengerEXT m_debugMessenger = {};

   inline static VkSwapchainKHR m_swapChain = {};
   inline static std::vector< VkImage > m_swapChainImages = {};
   inline static std::vector< VkImageView > m_swapChainImageViews = {};
   inline static std::vector< VkFramebuffer > m_swapChainFramebuffers = {};
   inline static VkFormat m_swapChainImageFormat = {};

   inline static VkDescriptorSetLayout m_descriptorSetLayout = {};
   inline static VkDescriptorPool m_descriptorPool = {};
   inline static std::vector< VkDescriptorSet > m_descriptorSets = {};

   inline static VkPipelineLayout m_pipelineLayout = {};

   inline static std::vector< VkCommandBuffer > m_commandBuffers = {};

   inline static std::vector< VkSemaphore > m_imageAvailableSemaphores = {};
   inline static std::vector< VkSemaphore > m_renderFinishedSemaphores = {};
   inline static std::vector< VkFence > m_inFlightFences = {};
   inline static std::vector< VkFence > m_imagesInFlight = {};

   inline static VkImage m_depthImage = {};
   inline static VkDeviceMemory m_depthImageMemory = {};
   inline static VkImageView m_depthImageView = {};

   inline static VkImage m_colorImage = {};
   inline static VkDeviceMemory m_colorImageMemory = {};
   inline static VkImageView m_colorImageView = {};

   inline static DeferredPipeline m_deferredPipeline = {};
   inline static uint32_t m_imageIndex = {};
};

} // namespace shady::render::vulkan
