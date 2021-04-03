#pragma once

#include "renderer_api.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

struct GLFWwindow;
namespace shady::render::opengl {

class OpenGLRendererAPI : public RendererAPI
{
 public:
   virtual ~OpenGLRendererAPI() = default;

   void
   Init() override;

   void
   SetDepthFunc(DepthFunc depthFunc) override;

   void
   EnableDepthTesting() override;

   void
   DisableDepthTesting() override;

   void
   SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

   void
   SetClearColor(const glm::vec4& color) override;

   void
   Clear() override;

   void
   DrawIndexed(const std::shared_ptr< VertexArray >& vertexArray, uint32_t indexCount = 0) override;

   void
   MultiDrawElemsIndirect(uint32_t drawCount, size_t offset = 0) override;

   void
   DrawLines(uint32_t count) override;

   void
   CheckForErrors() override;

   static void CreateInstance();

   static void InitializeVulkan(GLFWwindow* windowHandle);

  private:
    inline static VkInstance m_instance = {};
    inline static VkDebugUtilsMessengerEXT m_debugMessenger = {};
    inline static VkSurfaceKHR m_surface = {};
    inline static VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    inline static VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    inline static VkDevice m_device = {};

    inline static VkQueue m_graphicsQueue = {};
    inline static VkQueue m_presentQueue = {};

    inline static VkSwapchainKHR m_swapChain = {};
    inline static std::vector<VkImage> m_swapChainImages = {};
    inline static std::vector<VkImageView> m_swapChainImageViews = {};
    inline static std::vector<VkFramebuffer> m_swapChainFramebuffers = {};
    inline static VkFormat m_swapChainImageFormat = {};
    inline static VkExtent2D m_swapChainExtent = {};

    inline static VkRenderPass m_renderPass = {};
    inline static VkPipelineLayout m_pipelineLayout = {};
    inline static VkPipeline m_graphicsPipeline = {};

    inline static VkCommandPool m_commandPool = {};
    inline static std::vector<VkCommandBuffer> m_commandBuffers = {};

};

} // namespace shady::render::opengl
