#pragma once

#include "deferred_pipeline.hpp"
#include "render/vulkan/vertex.hpp"
#include "vulkan/types.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>


struct GLFWwindow;

namespace shady::render::vulkan {

class VulkanRenderer
{
 public:
   static void
   Initialize(GLFWwindow* windowHandle);

   static void
   CreateRenderPipeline();

   static void
   Draw();

   static void
   DrawDeferred();

   static void
   MeshLoaded(const std::vector< vulkan::Vertex >& vertices,
              const std::vector< uint32_t >& indicies, const TextureMaps& textures,
              const glm::mat4& modelMat);

   inline static glm::mat4 view_mat = glm::mat4(1.0f);
   inline static glm::mat4 proj_mat = glm::mat4(1.0f);
   inline static glm::vec4 camera_pos = {};
   inline static glm::vec4 light_pos = {};

   static void
   CreateCommandBufferForDeferred();

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
   CreateDescriptorSetLayout();

   static void
   CreateRenderPass();

   static void
   CreateCommandPool();

   static void
   CreateFramebuffers();

   static void
   CreateCommandBuffers();


   static void
   CreateSyncObjects();

   static void
   CreatePipelineCache();

   static void
   CreatePipeline();


   static void
   CreateVertexBuffer();

   static void
   CreateIndexBuffer();

   static void
   CreateUniformBuffers();

   static void
   UpdateUniformBuffer(uint32_t currentImage);

   static void
   CreateDescriptorPool();

   static void
   CreateDescriptorSets();


   static void
   CreateDepthResources();

   static void
   CreateColorResources();

   static bool
   HasStencilComponent(VkFormat format);

 private:
   inline static VkDebugUtilsMessengerCreateInfoEXT m_debugCreateInfo = {};
   inline static VkDebugUtilsMessengerEXT m_debugMessenger = {};

   inline static VkSwapchainKHR m_swapChain = {};
   inline static std::vector< VkImage > m_swapChainImages = {};
   inline static std::vector< VkImageView > m_swapChainImageViews = {};
   inline static std::vector< VkFramebuffer > m_swapChainFramebuffers = {};
   inline static VkFormat m_swapChainImageFormat = {};
   inline static VkExtent2D m_swapChainExtent = {};

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
};

} // namespace shady::render::vulkan
