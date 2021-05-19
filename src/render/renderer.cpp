
#include "renderer.hpp"
#include "app/gui/gui.hpp"
#include "buffer.hpp"
#include "command.hpp"
#include "common.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"
#include "utils/file_manager.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <optional>
#include <set>

#undef max
#undef min

namespace shady::render {

size_t currentFrame = 0;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
static bool is_deferred = true;

void
Renderer::MeshLoaded(const std::vector< Vertex >& vertices_in,
                     const std::vector< uint32_t >& indicies_in, const TextureMaps& textures_in,
                     const glm::mat4& modelMat)
{
   std::copy(vertices_in.begin(), vertices_in.end(), std::back_inserter(Data::vertices));
   std::copy(indicies_in.begin(), indicies_in.end(), std::back_inserter(Data::indices));

   VkDrawIndexedIndirectCommand newModel = {};
   newModel.firstIndex = Data::m_currentIndex;
   newModel.indexCount = static_cast< uint32_t >(indicies_in.size());
   newModel.firstInstance = 0;
   newModel.instanceCount = 1;
   newModel.vertexOffset = static_cast< int32_t >(Data::m_currentVertex);
   Data::m_renderCommands.push_back(newModel);

   Data::m_currentVertex += static_cast< uint32_t >(vertices_in.size());
   Data::m_currentIndex += static_cast< uint32_t >(indicies_in.size());

   PerInstanceBuffer newInstance;
   newInstance.model = modelMat;

   for (const auto& texture : textures_in)
   {
      if (texture.empty())
      {
         continue;
      }

      const auto it = std::find_if(Data::textures.begin(), Data::textures.end(),
                                   [texture](const auto& tex) { return texture == tex.first; });
      if (it == Data::textures.end())
      {
         Data::textures[texture] = {
            Data::currTexIdx++, TextureLibrary::GetTexture(texture).GetImageViewAndSampler().first};

         Data::texturesVec.push_back(
            TextureLibrary::GetTexture(texture).GetImageViewAndSampler().first);
      }

      const auto idx = Data::textures[texture].first;
      const auto tex = TextureLibrary::GetTexture(texture);
      switch (tex.GetType())
      {
         case TextureType::DIFFUSE_MAP: {
            newInstance.textures.x = static_cast< float >(idx);
         }
         break;

         case TextureType::NORMAL_MAP: {
            newInstance.textures.y = static_cast< float >(idx);
         }
         break;

         case TextureType::SPECULAR_MAP: {
            newInstance.textures.z = static_cast< float >(idx);
         }
         break;

         case TextureType::CUBE_MAP:
         default:
            break;
      }
   }

   Data::perInstance.push_back(newInstance);

   ++Data::m_numMeshes;
}

void
Renderer::SetupData()
{
   CreateVertexBuffer();
   CreateIndexBuffer();
   CreateUniformBuffers();

   const auto commandsSize = Data::m_renderCommands.size() * sizeof(VkDrawIndexedIndirectCommand);
   // VkBuffer stagingBuffer;
   // VkDeviceMemory stagingBufferMemory;

   ////  Commands + draw count
   VkDeviceSize bufferSize = commandsSize + sizeof(uint32_t);

   Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        Data::m_indirectDrawsBuffer, Data::m_indirectDrawsBufferMemory);

   void* data;
   vkMapMemory(Data::vk_device, Data::m_indirectDrawsBufferMemory, 0, bufferSize, 0, &data);
   memcpy(data, Data::m_renderCommands.data(), static_cast< size_t >(bufferSize));
   memcpy(static_cast< uint8_t* >(data) + commandsSize, &Data::m_numMeshes, sizeof(uint32_t));


   // vkUnmapMemory(Data::vk_device, stagingBufferMemory);

   /*  Buffer::CreateBuffer(
        bufferSize, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indirectDrawsBuffer, m_indirectDrawsBufferMemory);

     Buffer::CopyBuffer(stagingBuffer, m_indirectDrawsBuffer, bufferSize);

     vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
     vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);*/
}

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


/*
 *  Query GLFW for required extensions
 */
std::vector< const char* >
getRequiredExtensions()
{
   uint32_t glfwExtensionCount = 0;
   const char** glfwExtensions;
   glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

   std::vector< const char* > extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

   if constexpr (ENABLE_VALIDATION)
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

   std::set< std::string > validationLayers(VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end());

   for (const auto& extension : availableLayers)
   {
      validationLayers.erase(extension.layerName);
   }

   return validationLayers.empty();
}

void
populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
   auto callback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                      VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                      void* /*pUserData*/) -> VKAPI_ATTR VkBool32 {
      switch (messageSeverity)
      {
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
            trace::Logger::Debug("validation layer: {}", pCallbackData->pMessage);
         }
         break;

         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            trace::Logger::Info("validation layer: {}", pCallbackData->pMessage);
         }
         break;

         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            trace::Logger::Warn("validation layer: {}", pCallbackData->pMessage);
         }
         break;

         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            trace::Logger::Fatal("validation layer: {}", pCallbackData->pMessage);
         }
         break;

         default: {
            trace::Logger::Fatal("validation layer: {}", pCallbackData->pMessage);
         }
      }

      return VK_FALSE;
   };

   createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
   createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
   createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
   createInfo.pfnUserCallback = callback;
}

QueueFamilyIndices
findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
   QueueFamilyIndices indices;

   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

   std::vector< VkQueueFamilyProperties > queueFamilies(queueFamilyCount);
   vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

   uint32_t i = 0;
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

   std::set< std::string > requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

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
          && supportedFeatures.samplerAnisotropy && supportedFeatures.multiDrawIndirect
          && supportedFeatures.geometryShader;
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
      // VK_PRESENT_MODE_MAILBOX_KHR is the lowest latency non-tearing present mode available
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      {
         return availablePresentMode;
      }
   }

   // VK_PRESENT_MODE_FIFO_KHR is v-sync
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

      actualExtent.width = glm::clamp(actualExtent.width, capabilities.maxImageExtent.width,
                                      capabilities.minImageExtent.width);

      actualExtent.height = glm::clamp(actualExtent.height, capabilities.maxImageExtent.height,
                                       capabilities.minImageExtent.height);

      return actualExtent;
   }
}

void
Renderer::CreateVertexBuffer()
{
   VkDeviceSize bufferSize = sizeof(Data::vertices[0]) * Data::vertices.size();

   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;
   Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stagingBuffer, stagingBufferMemory);

   void* data;
   vkMapMemory(Data::vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
   memcpy(data, Data::vertices.data(), static_cast< size_t >(bufferSize));
   vkUnmapMemory(Data::vk_device, stagingBufferMemory);

   Buffer::CreateBuffer(
      bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Data::m_vertexBuffer, Data::m_vertexBufferMemory);

   Buffer::CopyBuffer(stagingBuffer, Data::m_vertexBuffer, bufferSize);

   vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
   vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
}

void
Renderer::CreateIndexBuffer()
{
   VkDeviceSize bufferSize = sizeof(Data::indices[0]) * Data::indices.size();

   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;
   Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stagingBuffer, stagingBufferMemory);

   void* data;
   vkMapMemory(Data::vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
   memcpy(data, Data::indices.data(), static_cast< size_t >(bufferSize));
   vkUnmapMemory(Data::vk_device, stagingBufferMemory);

   Buffer::CreateBuffer(
      bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Data::m_indexBuffer, Data::m_indexBufferMemory);

   Buffer::CopyBuffer(stagingBuffer, Data::m_indexBuffer, bufferSize);

   vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
   vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
}

void
Renderer::CreateUniformBuffers()
{
   VkDeviceSize bufferSize = sizeof(UniformBufferObject);
   VkDeviceSize SSBObufferSize = Data::perInstance.size() * sizeof(PerInstanceBuffer);

   const auto swapchainImagesSize = m_swapChainImages.size();

   Data::m_uniformBuffers.resize(swapchainImagesSize);
   Data::m_uniformBuffersMemory.resize(swapchainImagesSize);
   Data::m_ssbo.resize(swapchainImagesSize);
   Data::m_ssboMemory.resize(swapchainImagesSize);

   for (size_t i = 0; i < swapchainImagesSize; i++)
   {
      Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                              | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           Data::m_uniformBuffers[i], Data::m_uniformBuffersMemory[i]);

      Buffer::CreateBuffer(SSBObufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                              | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           Data::m_ssbo[i], Data::m_ssboMemory[i]);
   }
}
void
Renderer::CreateDescriptorPool()
{
   std::array< VkDescriptorPoolSize, 2 > poolSizes{};
   poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   poolSizes[0].descriptorCount = static_cast< uint32_t >(m_swapChainImages.size());
   poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   poolSizes[1].descriptorCount = static_cast< uint32_t >(m_swapChainImages.size());

   VkDescriptorPoolCreateInfo poolInfo{};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.poolSizeCount = static_cast< uint32_t >(poolSizes.size());
   poolInfo.pPoolSizes = poolSizes.data();
   poolInfo.maxSets = static_cast< uint32_t >(m_swapChainImages.size());

   if (vkCreateDescriptorPool(Data::vk_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
   {
      throw std::runtime_error("failed to create descriptor pool!");
   }
}

void
Renderer::CreateDescriptorSets()
{
   std::vector< VkDescriptorSetLayout > layouts(m_swapChainImages.size(), m_descriptorSetLayout);
   VkDescriptorSetAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocInfo.descriptorPool = m_descriptorPool;
   allocInfo.descriptorSetCount = static_cast< uint32_t >(m_swapChainImages.size());
   allocInfo.pSetLayouts = layouts.data();

   m_descriptorSets.resize(m_swapChainImages.size());
   if (vkAllocateDescriptorSets(Data::vk_device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS)
   {
      throw std::runtime_error("failed to allocate descriptor sets!");
   }

   const auto [imageView, sampler] =
      TextureLibrary::GetTexture(TextureType::DIFFUSE_MAP, "196.png").GetImageViewAndSampler();

   for (size_t i = 0; i < m_swapChainImages.size(); i++)
   {
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = Data::m_uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);


      VkDescriptorBufferInfo instanceBufferInfo;
      instanceBufferInfo.buffer = Data::m_ssbo[i];
      instanceBufferInfo.offset = 0;
      instanceBufferInfo.range = Data::perInstance.size() * sizeof(PerInstanceBuffer);

      /*VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = imageView;
      imageInfo.sampler = sampler;*/

      VkDescriptorImageInfo* descriptorImageInfos =
         new VkDescriptorImageInfo[Data::textures.size()];

      uint32_t texI = 0;
      for (const auto& texture : Data::texturesVec)
      {
         descriptorImageInfos[texI].sampler = nullptr;
         descriptorImageInfos[texI].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
         descriptorImageInfos[texI].imageView = texture;

         ++texI;
      }

      std::array< VkWriteDescriptorSet, 4 > descriptorWrites{};

      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = m_descriptorSets[i];
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = m_descriptorSets[i];
      descriptorWrites[1].dstBinding = 1;
      descriptorWrites[1].dstArrayElement = 0;
      descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      descriptorWrites[1].descriptorCount = 1;
      descriptorWrites[1].pBufferInfo = &instanceBufferInfo;

      VkDescriptorImageInfo samplerInfo = {};
      samplerInfo.sampler = sampler;

      descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[2].dstSet = m_descriptorSets[i];
      descriptorWrites[2].dstBinding = 2;
      descriptorWrites[2].dstArrayElement = 0;
      descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
      descriptorWrites[2].descriptorCount = 1;
      descriptorWrites[2].pImageInfo = &samplerInfo;

      descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[3].dstSet = m_descriptorSets[i];
      descriptorWrites[3].dstBinding = 3;
      descriptorWrites[3].dstArrayElement = 0;
      descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      descriptorWrites[3].descriptorCount = static_cast< uint32_t >(Data::textures.size());
      descriptorWrites[3].pImageInfo = descriptorImageInfos;

      vkUpdateDescriptorSets(Data::vk_device, static_cast< uint32_t >(descriptorWrites.size()),
                             descriptorWrites.data(), 0, nullptr);

      delete[] descriptorImageInfos;
   }
}

void
Renderer::Initialize(GLFWwindow* windowHandle)
{
   CreateInstance();

   utils::Assert(glfwCreateWindowSurface(Data::vk_instance, windowHandle, nullptr, &Data::m_surface)
                    == VK_SUCCESS,
                 "failed to create window surface!");

   CreateDevice();
   CreateSwapchain(windowHandle);
   CreateImageViews();
   CreateCommandPool();
}

void
Renderer::CreatePipelineCache()
{
   VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
   pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
   VK_CHECK(vkCreatePipelineCache(Data::vk_device, &pipelineCacheCreateInfo, nullptr,
                                  &Data::m_pipelineCache),
            "");
}

void
Renderer::CreateRenderPipeline()
{
   SetupData();

   CreateRenderPass();
   CreateColorResources();
   CreateDepthResources();
   CreateFramebuffers();
   CreatePipelineCache();

   if (!is_deferred)
   {
   }
   else
   {
      m_deferredPipeline.Initialize(Data::m_renderPass, m_swapChainImageViews,
                                    Data::m_pipelineCache);
      app::gui::Gui::Init({Data::m_swapChainExtent.width, Data::m_swapChainExtent.height});
     //  app::gui::Gui::UpdateUI({Data::m_swapChainExtent.width, Data::m_swapChainExtent.height});
      CreateCommandBufferForDeferred();
   }

   CreateSyncObjects();
}

void
Renderer::UpdateUniformBuffer(const scene::Camera* camera, const scene::Light* light)
{
   UniformBufferObject ubo{};

   ubo.proj = camera->GetViewProjection();
   ubo.lightView = light->GetLightSpaceMat();

   void* data;
   vkMapMemory(Data::vk_device, Data::m_uniformBuffersMemory[m_imageIndex], 0, sizeof(ubo), 0,
               &data);
   memcpy(data, &ubo, sizeof(ubo));
   vkUnmapMemory(Data::vk_device, Data::m_uniformBuffersMemory[m_imageIndex]);

   void* data2;
   vkMapMemory(Data::vk_device, Data::m_ssboMemory[m_imageIndex], 0,
               Data::perInstance.size() * sizeof(PerInstanceBuffer), 0, &data2);
   memcpy(data2, Data::perInstance.data(), Data::perInstance.size() * sizeof(PerInstanceBuffer));
   vkUnmapMemory(Data::vk_device, Data::m_ssboMemory[m_imageIndex]);

   m_deferredPipeline.UpdateDeferred(camera, light);
}

void
Renderer::CreateColorResources()
{
   VkFormat colorFormat = m_swapChainImageFormat;

   std::tie(m_colorImage, m_colorImageMemory) = Texture::CreateImage(
      Data::m_swapChainExtent.width, Data::m_swapChainExtent.height, 1,
      VK_SAMPLE_COUNT_1_BIT /*Data::m_msaaSamples*/, colorFormat, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
   m_colorImageView =
      Texture::CreateImageView(m_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void
Renderer::CreateDepthResources()
{
   VkFormat depthFormat = FindDepthFormat();

   const auto [depthImage, depthImageMemory] = Texture::CreateImage(
      Data::m_swapChainExtent.width, Data::m_swapChainExtent.height, 1,
      VK_SAMPLE_COUNT_1_BIT /*Data::m_msaaSamples*/, depthFormat, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

   m_depthImage = depthImage;
   m_depthImageMemory = depthImageMemory;

   m_depthImageView =
      Texture::CreateImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

bool
Renderer::HasStencilComponent(VkFormat format)
{
   return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void
Renderer::Draw()
{
   // Always recreate the command buffers for composition, mostly due to imgui
   CreateCommandBufferForDeferred();

   // vkWaitForFences(Data::vk_device, 1, &m_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

   vkAcquireNextImageKHR(Data::vk_device, m_swapChain, UINT64_MAX,
                         m_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &m_imageIndex);

   // UpdateUniformBuffer();
   // if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
   //{
   //   vkWaitForFences(Data::vk_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
   //}
   // m_imagesInFlight[imageIndex] = m_inFlightFences[currentFrame];


   //
   // Offscreen rendering
   //

   VkSubmitInfo submitInfo{};
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
   submitInfo.pWaitDstStageMask = waitStages;

   // Wait for swap chain presentation to finish
   submitInfo.pWaitSemaphores = &m_imageAvailableSemaphores[currentFrame];
   submitInfo.waitSemaphoreCount = 1;

   // Signal ready with offscreen semaphore
   submitInfo.pSignalSemaphores = &m_deferredPipeline.GetOffscreenSemaphore();
   submitInfo.signalSemaphoreCount = 1;

   // Submit work
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &m_deferredPipeline.GetOffscreenCmdBuffer();
   VK_CHECK(vkQueueSubmit(Data::vk_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE),
            "failed to submit offscreen draw command buffer!");


   //
   // Scene rendering
   //

   submitInfo.pWaitSemaphores = &m_deferredPipeline.GetOffscreenSemaphore();

   submitInfo.pSignalSemaphores = &m_renderFinishedSemaphores[currentFrame];

   submitInfo.pCommandBuffers = &m_commandBuffers[m_imageIndex];
   submitInfo.commandBufferCount = 1;


   // vkResetFences(Data::vk_device, 1, &m_inFlightFences[currentFrame]);

   // VK_CHECK(vkQueueSubmit(Data::vk_graphicsQueue, 1, &submitInfo,
   // m_inFlightFences[currentFrame]),
   //         "failed to submit draw command buffer!");

   VK_CHECK(vkQueueSubmit(Data::vk_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE),
            "failed to submit draw command buffer!");

   VkPresentInfoKHR presentInfo{};
   presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[currentFrame];

   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = &m_swapChain;

   presentInfo.pImageIndices = &m_imageIndex;

   vkQueuePresentKHR(Data::m_presentQueue, &presentInfo);

   vkQueueWaitIdle(Data::vk_graphicsQueue);

   currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void
Renderer::CreateInstance()
{
   if (ENABLE_VALIDATION && !checkValidationLayerSupport())
   {
      utils::Assert(false, "validation layers asked but not supported!");
   }

   VkApplicationInfo appInfo{};
   appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   appInfo.pApplicationName = "Shady";
   appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
   appInfo.apiVersion = VK_API_VERSION_1_2;

   VkInstanceCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   createInfo.pApplicationInfo = &appInfo;

   const auto extensions = getRequiredExtensions();
   createInfo.enabledExtensionCount = static_cast< uint32_t >(extensions.size());
   createInfo.ppEnabledExtensionNames = extensions.data();

   if constexpr (ENABLE_VALIDATION)
   {
      createInfo.enabledLayerCount = static_cast< uint32_t >(VALIDATION_LAYERS.size());
      createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

      populateDebugMessengerCreateInfo(m_debugCreateInfo);
      createInfo.pNext = reinterpret_cast< void* >(&m_debugCreateInfo);
   }

   VK_CHECK(vkCreateInstance(&createInfo, nullptr, &Data::vk_instance),
            "instance not created properly!");
}

void
Renderer::CreateDevice()
{
   uint32_t deviceCount = 0;
   vkEnumeratePhysicalDevices(Data::vk_instance, &deviceCount, nullptr);
   utils::Assert(deviceCount > 0, "failed to find GPUs with Vulkan support!");

   std::vector< VkPhysicalDevice > devices(deviceCount);
   vkEnumeratePhysicalDevices(Data::vk_instance, &deviceCount, devices.data());

   for (const auto& device : devices)
   {
      if (isDeviceSuitable(device, Data::m_surface))
      {
         VkPhysicalDeviceProperties deviceProps{};
         vkGetPhysicalDeviceProperties(device, &deviceProps);
         trace::Logger::Info("Device found! Using {}", deviceProps.deviceName);

         Data::vk_physicalDevice = device;
         Data::m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
         // getMaxUsableSampleCount(Data::vk_physicalDevice);
         break;
      }
   }

   utils::Assert(Data::vk_physicalDevice != VK_NULL_HANDLE, "failed to find a suitable GPU!");


   QueueFamilyIndices indices = findQueueFamilies(Data::vk_physicalDevice, Data::m_surface);

   std::vector< VkDeviceQueueCreateInfo > queueCreateInfos;
   std::set< uint32_t > uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                               indices.presentFamily.value()};

   float queuePriority = 1.0f;
   for (auto queueFamily : uniqueQueueFamilies)
   {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;

      queueCreateInfos.push_back(queueCreateInfo);
   }

   VkPhysicalDeviceVulkan12Features deviceFeatures_12{};
   deviceFeatures_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
   deviceFeatures_12.drawIndirectCount = VK_TRUE;

   VkPhysicalDeviceFeatures deviceFeatures{};
   deviceFeatures.samplerAnisotropy = VK_TRUE;
   deviceFeatures.multiDrawIndirect = VK_TRUE;
   deviceFeatures.geometryShader = VK_TRUE;

   VkDeviceCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   createInfo.pNext = &deviceFeatures_12;

   createInfo.queueCreateInfoCount = static_cast< uint32_t >(queueCreateInfos.size());
   createInfo.pQueueCreateInfos = queueCreateInfos.data();

   createInfo.pEnabledFeatures = &deviceFeatures;

   createInfo.enabledExtensionCount = static_cast< uint32_t >(DEVICE_EXTENSIONS.size());
   createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

   if constexpr (ENABLE_VALIDATION)
   {
      createInfo.enabledLayerCount = static_cast< uint32_t >(VALIDATION_LAYERS.size());
      createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
   }

   VK_CHECK(vkCreateDevice(Data::vk_physicalDevice, &createInfo, nullptr, &Data::vk_device),
            "failed to create logical device!");

   vkGetDeviceQueue(Data::vk_device, indices.graphicsFamily.value(), 0, &Data::vk_graphicsQueue);
   vkGetDeviceQueue(Data::vk_device, indices.presentFamily.value(), 0, &Data::m_presentQueue);
}

void
Renderer::CreateSwapchain(GLFWwindow* windowHandle)
{
   SwapChainSupportDetails swapChainSupport =
      querySwapChainSupport(Data::vk_physicalDevice, Data::m_surface);

   VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
   VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
   VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, windowHandle);

   uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
   if (swapChainSupport.capabilities.maxImageCount > 0
       && imageCount > swapChainSupport.capabilities.maxImageCount)
   {
      imageCount = swapChainSupport.capabilities.maxImageCount;
   }


   VkSwapchainCreateInfoKHR swapChainCreateInfo{};
   swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   swapChainCreateInfo.surface = Data::m_surface;

   swapChainCreateInfo.minImageCount = imageCount;
   swapChainCreateInfo.imageFormat = surfaceFormat.format;
   swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
   swapChainCreateInfo.imageExtent = extent;
   swapChainCreateInfo.imageArrayLayers = 1;
   swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

   QueueFamilyIndices indicesSecond = findQueueFamilies(Data::vk_physicalDevice, Data::m_surface);
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

   VK_CHECK(vkCreateSwapchainKHR(Data::vk_device, &swapChainCreateInfo, nullptr, &m_swapChain),
            "failed to create swap chain!");

   vkGetSwapchainImagesKHR(Data::vk_device, m_swapChain, &imageCount, nullptr);
   m_swapChainImages.resize(imageCount);
   vkGetSwapchainImagesKHR(Data::vk_device, m_swapChain, &imageCount, m_swapChainImages.data());

   m_swapChainImageFormat = surfaceFormat.format;
   Data::m_swapChainExtent = extent;
}

void
Renderer::CreateImageViews()
{
   m_swapChainImageViews.resize(m_swapChainImages.size());

   for (uint32_t i = 0; i < m_swapChainImages.size(); i++)
   {
      m_swapChainImageViews[i] = Texture::CreateImageView(
         m_swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
   }
}

void
Renderer::CreateDescriptorSetLayout()
{
   VkDescriptorSetLayoutBinding uboLayoutBinding{};
   uboLayoutBinding.binding = 0;
   uboLayoutBinding.descriptorCount = 1;
   uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   uboLayoutBinding.pImmutableSamplers = nullptr;
   uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

   VkDescriptorSetLayoutBinding perInstanceBinding{};
   perInstanceBinding.binding = 1;
   perInstanceBinding.descriptorCount = 1;
   perInstanceBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
   perInstanceBinding.pImmutableSamplers = nullptr;
   perInstanceBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

   VkDescriptorSetLayoutBinding samplerLayoutBinding{};
   samplerLayoutBinding.binding = 2;
   samplerLayoutBinding.descriptorCount = 1;
   samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
   samplerLayoutBinding.pImmutableSamplers = nullptr;
   samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

   VkDescriptorSetLayoutBinding texturesLayoutBinding{};
   texturesLayoutBinding.binding = 3;
   texturesLayoutBinding.descriptorCount = static_cast< uint32_t >(Data::textures.size());
   texturesLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
   texturesLayoutBinding.pImmutableSamplers = nullptr;
   texturesLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

   std::array< VkDescriptorSetLayoutBinding, 4 > bindings = {
      uboLayoutBinding, perInstanceBinding, samplerLayoutBinding, texturesLayoutBinding};

   VkDescriptorSetLayoutCreateInfo layoutInfo{};
   layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   layoutInfo.bindingCount = static_cast< uint32_t >(bindings.size());
   layoutInfo.pBindings = bindings.data();

   VK_CHECK(
      vkCreateDescriptorSetLayout(Data::vk_device, &layoutInfo, nullptr, &m_descriptorSetLayout),
      "Failed to create descriptor set layout!");
}

void
Renderer::CreateRenderPass()
{
   VkAttachmentDescription colorAttachment{};
   colorAttachment.format = m_swapChainImageFormat;
   colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
   // colorAttachment.samples = Data::m_msaaSamples;
   colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
   colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

   VkAttachmentDescription depthAttachment{};
   depthAttachment.format = FindDepthFormat();
   depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
   // depthAttachment.samples = Data::m_msaaSamples;
   depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

   /*VkAttachmentDescription colorAttachmentResolve{};
   colorAttachmentResolve.format = m_swapChainImageFormat;
   colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
   colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
   colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;*/

   VkAttachmentReference colorAttachmentRef{};
   colorAttachmentRef.attachment = 0;
   colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

   VkAttachmentReference depthAttachmentRef{};
   depthAttachmentRef.attachment = 1;
   depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

   /*VkAttachmentReference colorAttachmentResolveRef{};
   colorAttachmentResolveRef.attachment = 2;
   colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;*/

   VkSubpassDescription subpass{};
   subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.colorAttachmentCount = 1;
   subpass.pColorAttachments = &colorAttachmentRef;
   subpass.pDepthStencilAttachment = &depthAttachmentRef;
   // subpass.pResolveAttachments = &colorAttachmentResolveRef;

   std::array< VkSubpassDependency, 2 > dependencies;

   dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
   dependencies[0].dstSubpass = 0;
   dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
   dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
   dependencies[0].dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
   dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

   dependencies[1].srcSubpass = 0;
   dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
   dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
   dependencies[1].srcAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
   dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
   dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

   // VkSubpassDependency dependency{};
   // dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
   // dependency.dstSubpass = 0;
   // dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   // dependency.srcAccessMask = 0;
   // dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   // dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

   std::array< VkAttachmentDescription, 2 > attachments = {colorAttachment, depthAttachment,
                                                           /*colorAttachmentResolve*/};
   VkRenderPassCreateInfo renderPassInfo{};
   renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   renderPassInfo.attachmentCount = static_cast< uint32_t >(attachments.size());
   renderPassInfo.pAttachments = attachments.data();
   renderPassInfo.subpassCount = 1;
   renderPassInfo.pSubpasses = &subpass;
   renderPassInfo.dependencyCount = static_cast< uint32_t >(dependencies.size());
   renderPassInfo.pDependencies = dependencies.data();

   VK_CHECK(vkCreateRenderPass(Data::vk_device, &renderPassInfo, nullptr, &Data::m_renderPass),
            "failed to create render pass!");
}

void
Renderer::CreateFramebuffers()
{
   m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

   for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
   {
      std::array< VkImageView, 2 > attachments = {m_swapChainImageViews[i], m_depthImageView,
                                                  /*m_swapChainImageViews[i]*/};


      VkFramebufferCreateInfo framebufferInfo{};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = Data::m_renderPass;
      framebufferInfo.attachmentCount = static_cast< uint32_t >(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = Data::m_swapChainExtent.width;
      framebufferInfo.height = Data::m_swapChainExtent.height;
      framebufferInfo.layers = 1;

      VK_CHECK(vkCreateFramebuffer(Data::vk_device, &framebufferInfo, nullptr,
                                   &m_swapChainFramebuffers[i]),
               "failed to create framebuffer!");
   }
}

void
Renderer::CreateCommandPool()
{
   QueueFamilyIndices queueFamilyIndicesTwo =
      findQueueFamilies(Data::vk_physicalDevice, Data::m_surface);

   VkCommandPoolCreateInfo poolInfo{};
   poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   poolInfo.queueFamilyIndex = queueFamilyIndicesTwo.graphicsFamily.value();

   VK_CHECK(vkCreateCommandPool(Data::vk_device, &poolInfo, nullptr, &Data::vk_commandPool),
            "failed to create command pool!");
}

void
Renderer::CreateCommandBuffers()
{
   CreateDescriptorPool();
   CreateDescriptorSets();

   /*
    *  CREATE COMMAND BUFFERS
    */

   m_commandBuffers.resize(m_swapChainFramebuffers.size());

   VkCommandBufferAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   allocInfo.commandPool = Data::vk_commandPool;
   allocInfo.commandBufferCount = static_cast< uint32_t >(m_commandBuffers.size());

   VK_CHECK(vkAllocateCommandBuffers(Data::vk_device, &allocInfo, m_commandBuffers.data()),
            "failed to allocate command buffers!");

   VkCommandBufferBeginInfo beginInfo{};
   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

   std::array< VkClearValue, 2 > clearValues{};
   clearValues[0].color = {{0.3f, 0.5f, 0.1f, 1.0f}};
   clearValues[1].depthStencil = {1.0f, 0};

   VkRenderPassBeginInfo renderPassInfo{};
   renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   renderPassInfo.renderPass = Data::m_renderPass;
   renderPassInfo.renderArea.offset = {0, 0};
   renderPassInfo.renderArea.extent = Data::m_swapChainExtent;
   renderPassInfo.clearValueCount = static_cast< uint32_t >(clearValues.size());
   renderPassInfo.pClearValues = clearValues.data();

   for (uint32_t i = 0; i < m_commandBuffers.size(); i++)
   {
      VK_CHECK(vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo),
               "failed to begin recording command buffer!");

      renderPassInfo.framebuffer = m_swapChainFramebuffers[i];

      vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

      vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                        Data::m_graphicsPipeline);

      VkBuffer vertexBuffers[] = {Data::m_vertexBuffer};
      VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);

      vkCmdBindIndexBuffer(m_commandBuffers[i], Data::m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);


      /*vkCmdDrawIndexedIndirect(m_commandBuffers[i], m_indirectDrawsBuffer, 0, m_numMeshes,
                        sizeof(VkDrawIndexedIndirectCommand));*/

      // This allows for dynamic number of meshes in the scene
      // For static scenes, vkCmdDrawIndexedIndirect should probably be used with DEVICE_LOCAL
      // memory type
      vkCmdDrawIndexedIndirectCount(m_commandBuffers[i], Data::m_indirectDrawsBuffer, 0,
                                    Data::m_indirectDrawsBuffer,
                                    sizeof(VkDrawIndexedIndirectCommand) * Data::m_numMeshes,
                                    Data::m_numMeshes, sizeof(VkDrawIndexedIndirectCommand));

      vkCmdEndRenderPass(m_commandBuffers[i]);

      VK_CHECK(vkEndCommandBuffer(m_commandBuffers[i]), "failed to record command buffer!");
   }
}

void
Renderer::CreateCommandBufferForDeferred()
{
   if (m_commandBuffers.empty())
   {
      m_commandBuffers.resize(m_swapChainFramebuffers.size());

      VkCommandBufferAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandPool = Data::vk_commandPool;
      allocInfo.commandBufferCount = static_cast< uint32_t >(m_commandBuffers.size());

      VK_CHECK(vkAllocateCommandBuffers(Data::vk_device, &allocInfo, m_commandBuffers.data()),
               "failed to allocate command buffers!");
   }

   VkCommandBufferBeginInfo beginInfo{};
   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

   std::array< VkClearValue, 2 > clearValues{};
   clearValues[0].color = {{0.3f, 0.5f, 0.1f, 1.0f}};
   clearValues[1].depthStencil = {1.0f, 0};

   VkRenderPassBeginInfo renderPassInfo{};
   renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   renderPassInfo.renderPass = Data::m_renderPass;
   renderPassInfo.renderArea.offset = {0, 0};
   renderPassInfo.renderArea.extent = Data::m_swapChainExtent;
   renderPassInfo.clearValueCount = static_cast< uint32_t >(clearValues.size());
   renderPassInfo.pClearValues = clearValues.data();

   for (uint32_t i = 0; i < m_commandBuffers.size(); ++i)
   {
      renderPassInfo.framebuffer = m_swapChainFramebuffers[i];

      VK_CHECK(vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo), "");

      vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

      VkViewport viewport{};
      viewport.width = static_cast<float>(Data::m_swapChainExtent.width);
      viewport.height = static_cast<float>(Data::m_swapChainExtent.height);
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      vkCmdSetViewport(m_commandBuffers[i], 0, 1, &viewport);

      VkRect2D scissor{};
      scissor.extent.width = Data::m_swapChainExtent.width;
      scissor.extent.height = Data::m_swapChainExtent.height;
      scissor.offset.x = 0;
      scissor.offset.y = 0;

      vkCmdSetScissor(m_commandBuffers[i], 0, 1, &scissor);

      /*
       * STAGE 2 - COMPOSITION
       */
      vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_deferredPipeline.GetPipelineLayout(), 0, 1,
                              &m_deferredPipeline.GetDescriptorSet(), 0, nullptr);

      vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                        m_deferredPipeline.GetCompositionPipeline());

      // Final composition as full screen quad
      vkCmdDraw(m_commandBuffers[i], 3, 1, 0, 0);

      /*
       * STAGE 3 - DRAW UI
       */
      app::gui::Gui::Render(m_commandBuffers[i]);

      vkCmdEndRenderPass(m_commandBuffers[i]);

      VK_CHECK(vkEndCommandBuffer(m_commandBuffers[i]), "");
   }
}

void
Renderer::CreateSyncObjects()
{
   m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
   m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
   m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
   m_imagesInFlight.resize(m_swapChainImages.size(), VK_NULL_HANDLE);

   VkSemaphoreCreateInfo semaphoreInfo{};
   semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

   VkFenceCreateInfo fenceInfo{};
   fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
   fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

   for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
   {
      VK_CHECK(vkCreateSemaphore(Data::vk_device, &semaphoreInfo, nullptr,
                                 &m_imageAvailableSemaphores[i]),
               fmt::format("Failed to create m_imageAvailableSemaphores[{}]!", i));
      VK_CHECK(vkCreateSemaphore(Data::vk_device, &semaphoreInfo, nullptr,
                                 &m_renderFinishedSemaphores[i]),
               fmt::format("Failed to create m_renderFinishedSemaphores[{}]!", i));
      VK_CHECK(vkCreateFence(Data::vk_device, &fenceInfo, nullptr, &m_inFlightFences[i]),
               fmt::format("Failed to create m_inFlightFences[{}]!", i));
   }
}

void
Renderer::CreatePipeline()
{
   auto [vertexInfo, fragmentInfo] =
      Shader::CreateShader(Data::vk_device, "default/vert.spv", "default/frag.spv");
   VkPipelineShaderStageCreateInfo shaderStages[] = {vertexInfo.shaderInfo,
                                                     fragmentInfo.shaderInfo};

   VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
   vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   auto bindingDescription = Vertex::getBindingDescription();
   auto attributeDescriptions = Vertex::getAttributeDescriptions();
   vertexInputInfo.vertexBindingDescriptionCount = 1;
   vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast< uint32_t >(attributeDescriptions.size());
   vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
   vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

   VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
   inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
   inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
   inputAssembly.primitiveRestartEnable = VK_FALSE;

   VkViewport viewport{};
   viewport.x = 0.0f;
   viewport.y = 0.0f;
   viewport.width = static_cast< float >(Data::m_swapChainExtent.width);
   viewport.height = static_cast< float >(Data::m_swapChainExtent.height);
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;

   VkRect2D scissor{};
   scissor.offset = {0, 0};
   scissor.extent = Data::m_swapChainExtent;

   VkPipelineViewportStateCreateInfo viewportState{};
   viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
   viewportState.viewportCount = 1;
   viewportState.pViewports = &viewport;
   viewportState.scissorCount = 1;
   viewportState.pScissors = &scissor;

   VkPipelineRasterizationStateCreateInfo rasterizer{};
   rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
   rasterizer.depthClampEnable = VK_FALSE;
   rasterizer.rasterizerDiscardEnable = VK_FALSE;
   rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
   rasterizer.lineWidth = 1.0f;
   rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
   rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
   rasterizer.depthBiasEnable = VK_FALSE;

   VkPipelineMultisampleStateCreateInfo multisampling{};
   multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   multisampling.sampleShadingEnable = VK_FALSE;
   multisampling.rasterizationSamples = Data::m_msaaSamples;

   VkPipelineDepthStencilStateCreateInfo depthStencil{};
   depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
   depthStencil.depthTestEnable = VK_TRUE;
   depthStencil.depthWriteEnable = VK_TRUE;
   depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
   depthStencil.depthBoundsTestEnable = VK_FALSE;
   depthStencil.stencilTestEnable = VK_FALSE;

   VkPipelineColorBlendAttachmentState colorBlendAttachment{};
   colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                         | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   colorBlendAttachment.blendEnable = VK_FALSE;

   VkPipelineColorBlendStateCreateInfo colorBlending{};
   colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   colorBlending.logicOpEnable = VK_FALSE;
   colorBlending.logicOp = VK_LOGIC_OP_COPY;
   colorBlending.attachmentCount = 1;
   colorBlending.pAttachments = &colorBlendAttachment;
   colorBlending.blendConstants[0] = 0.0f;
   colorBlending.blendConstants[1] = 0.0f;
   colorBlending.blendConstants[2] = 0.0f;
   colorBlending.blendConstants[3] = 0.0f;

   VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
   pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutInfo.setLayoutCount = 1;
   pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;


   VK_CHECK(
      vkCreatePipelineLayout(Data::vk_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout),
      "failed to create pipeline layout!");

   VkGraphicsPipelineCreateInfo pipelineInfo{};
   pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineInfo.stageCount = 2;
   pipelineInfo.pStages = shaderStages;
   pipelineInfo.pVertexInputState = &vertexInputInfo;
   pipelineInfo.pInputAssemblyState = &inputAssembly;
   pipelineInfo.pViewportState = &viewportState;
   pipelineInfo.pRasterizationState = &rasterizer;
   pipelineInfo.pMultisampleState = &multisampling;
   pipelineInfo.pDepthStencilState = &depthStencil;
   pipelineInfo.pColorBlendState = &colorBlending;
   pipelineInfo.layout = m_pipelineLayout;
   pipelineInfo.renderPass = Data::m_renderPass;
   pipelineInfo.subpass = 0;
   pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;


   VK_CHECK(vkCreateGraphicsPipelines(Data::vk_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                      &Data::m_graphicsPipeline),
            "failed to create graphics pipeline!");

   // Shader info can be destroyed after the pipeline is created
   vertexInfo.Destroy();
   fragmentInfo.Destroy();
}

} // namespace shady::render
