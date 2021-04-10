#pragma once

#include "shader.hpp"

#include "render/vulkan/vertex.hpp"
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

   static void
   MeshLoaded(const std::vector< vulkan::Vertex >& vertices,
              const std::vector< uint32_t >& indicies);

   inline static glm::mat4 view_mat = glm::mat4(1.0f);
   inline static glm::mat4 proj_mat = glm::mat4(1.0f);

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
   CreateTextureImageView();

   static void
   CreateTextureSampler();

   static VkImageView
   CreateImageView(VkImage image, VkFormat format);

   static void
   CreateDepthResources();

   static void
   CreateColorResources();

   static VkFormat
   FindSupportedFormat(const std::vector< VkFormat >& candidates, VkImageTiling tiling,
                       VkFormatFeatureFlags features);

   static VkFormat
   FindDepthFormat();

   static bool
   HasStencilComponent(VkFormat format);

   //  static void
   //  LoadModel();

   // static glm::mat4 model_mat;


 private:
   inline static VkDebugUtilsMessengerCreateInfoEXT m_debugCreateInfo = {};
   inline static VkDebugUtilsMessengerEXT m_debugMessenger = {};
   inline static VkSurfaceKHR m_surface = {};

   inline static VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

   inline static VkQueue m_presentQueue = {};

   inline static VkSwapchainKHR m_swapChain = {};
   inline static std::vector< VkImage > m_swapChainImages = {};
   inline static std::vector< VkImageView > m_swapChainImageViews = {};
   inline static std::vector< VkFramebuffer > m_swapChainFramebuffers = {};
   inline static VkFormat m_swapChainImageFormat = {};
   inline static VkExtent2D m_swapChainExtent = {};

   inline static VkRenderPass m_renderPass = {};

   inline static VkDescriptorSetLayout m_descriptorSetLayout = {};
   inline static VkDescriptorPool m_descriptorPool = {};
   inline static std::vector< VkDescriptorSet > m_descriptorSets = {};

   inline static VkPipelineLayout m_pipelineLayout = {};
   inline static VkPipeline m_graphicsPipeline = {};


   inline static std::vector< VkCommandBuffer > m_commandBuffers = {};

   inline static std::vector< VkSemaphore > m_imageAvailableSemaphores = {};
   inline static std::vector< VkSemaphore > m_renderFinishedSemaphores = {};
   inline static std::vector< VkFence > m_inFlightFences = {};
   inline static std::vector< VkFence > m_imagesInFlight = {};

   inline static VkBuffer m_vertexBuffer = {};
   inline static VkDeviceMemory m_vertexBufferMemory = {};
   inline static VkBuffer m_indexBuffer = {};
   inline static VkDeviceMemory m_indexBufferMemory = {};

   inline static std::vector< VkBuffer > m_uniformBuffers = {};
   inline static std::vector< VkDeviceMemory > m_uniformBuffersMemory = {};

   inline static VkImage m_depthImage = {};
   inline static VkDeviceMemory m_depthImageMemory = {};
   inline static VkImageView m_depthImageView = {};

   inline static VkImage m_colorImage = {};
   inline static VkDeviceMemory m_colorImageMemory = {};
   inline static VkImageView m_colorImageView = {};
};

} // namespace shady::render::vulkan
