#pragma once

#include "scene/camera.hpp"
#include "scene/light.hpp"
#include "types.hpp"
#include "utils/assert.hpp"
#include "vertex.hpp"

#include <array>
#include <fmt/format.h>
#include <glm/glm.hpp>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>


namespace shady::render {

// This should make use of <source_location> when it's supported by clang/mvcc
constexpr void
VK_CHECK(VkResult result, std::string_view errorMessage)
{
   if (result != VK_SUCCESS)
   {
      utils::Assert(false,
                    fmt::format("{} Return value {}", errorMessage, string_VkResult(result)));
   }
}

static constexpr bool ENABLE_VALIDATION = true;
static constexpr std::array< const char*, 1 > VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
static constexpr std::array< const char*, 1 > DEVICE_EXTENSIONS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

/*
 * This is storage for common Vulkan objects that are needed for numerous
 * function calls. All these fields are assigned by Renderer on Init
 * and should only be read from other modules.
 */
struct Data
{
   inline static VkInstance vk_instance = {};
   inline static VkDevice vk_device = {};
   inline static VkPhysicalDevice vk_physicalDevice = VK_NULL_HANDLE;
   inline static VkQueue vk_graphicsQueue = {};
   inline static VkQueue m_presentQueue = {};
   inline static VkExtent2D m_swapChainExtent = {};
   inline static VkExtent2D m_deferredExtent = {};
   inline static VkCommandPool vk_commandPool = {};
   inline static VkSurfaceKHR m_surface = {};

   inline static VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

   inline static std::vector< VkDrawIndexedIndirectCommand > m_renderCommands = {};
   inline static VkBuffer m_indirectDrawsBuffer = {};
   inline static VkDeviceMemory m_indirectDrawsBufferMemory = {};
   inline static uint32_t m_currentVertex = {};
   inline static uint32_t m_currentIndex = {};
   inline static uint32_t m_numMeshes = {};

   inline static std::vector< VkBuffer > m_ssbo = {};
   inline static std::vector< VkDeviceMemory > m_ssboMemory = {};

   inline static std::vector< VkBuffer > m_uniformBuffers = {};
   inline static std::vector< VkDeviceMemory > m_uniformBuffersMemory = {};

   inline static VkBuffer m_vertexBuffer = {};
   inline static VkDeviceMemory m_vertexBufferMemory = {};
   inline static VkBuffer m_indexBuffer = {};
   inline static VkDeviceMemory m_indexBufferMemory = {};

   inline static std::vector< PerInstanceBuffer > perInstance;
   inline static std::vector< Vertex > vertices;
   inline static std::vector< uint32_t > indices;
   inline static int32_t currTexIdx = 0;
   inline static std::unordered_map< std::string, std::pair< int32_t, VkImageView > > textures = {};
   inline static std::vector< VkImageView > texturesVec = {};

   inline static VkPipelineCache m_pipelineCache = {};
   inline static VkPipeline m_graphicsPipeline = {};
   inline static VkRenderPass m_renderPass = {};
   inline static VkRenderPass m_deferredRenderPass = {};

   inline static DebugData m_debugData = {};
};

uint32_t
FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

VkFormat
FindSupportedFormat(const std::vector< VkFormat >& candidates, VkImageTiling tiling,
                    VkFormatFeatureFlags features);

VkFormat
FindDepthFormat();

} // namespace shady::render
