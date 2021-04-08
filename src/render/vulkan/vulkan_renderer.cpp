
#include "vulkan_renderer.hpp"
#include "trace/logger.hpp"
#include "utils/assert.hpp"
#include "utils/file_manager.hpp"
#include "vulkan_shader.hpp"
#include "vulkan_texture.hpp"
#include "vulkan_buffer.hpp"
#include "vulkan_command.hpp"
#include "vulkan_common.hpp"


#include <GLFW/glfw3.h>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <optional>
#include <set>

#undef max
#undef min

namespace shady::render::vulkan {

size_t currentFrame = 0;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject
{
   alignas(16) glm::mat4 model;
   alignas(16) glm::mat4 view;
   alignas(16) glm::mat4 proj;
};

struct Vertexx
{
   glm::vec2 pos;
   glm::vec3 color;

   static VkVertexInputBindingDescription
   getBindingDescription()
   {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Vertexx);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
   }

   static std::array< VkVertexInputAttributeDescription, 2 >
   getAttributeDescriptions()
   {
      std::array< VkVertexInputAttributeDescription, 2 > attributeDescriptions{};

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Vertexx, pos);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Vertexx, color);

      return attributeDescriptions;
   }
};

static const std::array< Vertexx, 4 > vertices = {
   Vertexx{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, Vertexx{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
   Vertexx{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, Vertexx{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

static const std::vector< uint16_t > indices = {0, 1, 2, 2, 3, 0};

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
                      void* /*pUserData*/) -> VKAPI_ATTR VkBool32 VKAPI_CALL {
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

      actualExtent.width = glm::clamp(actualExtent.width, capabilities.maxImageExtent.width,
                                      capabilities.minImageExtent.width);

      actualExtent.height = glm::clamp(actualExtent.height, capabilities.maxImageExtent.height,
                                       capabilities.minImageExtent.height);

      return actualExtent;
   }
}

void
VulkanRenderer::CreateVertexBuffer()
{
   VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;
   Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory);

   void* data;
   vkMapMemory(Data::vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
   memcpy(data, vertices.data(), (size_t)bufferSize);
   vkUnmapMemory(Data::vk_device, stagingBufferMemory);

   Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);

   Buffer::CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

   vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
   vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
}

void
VulkanRenderer::CreateIndexBuffer()
{
   VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;
   Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory);

   void* data;
   vkMapMemory(Data::vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
   memcpy(data, indices.data(), (size_t)bufferSize);
   vkUnmapMemory(Data::vk_device, stagingBufferMemory);

   Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

   Buffer::CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

   vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
   vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
}

void
VulkanRenderer::CreateUniformBuffers()
{
   VkDeviceSize bufferSize = sizeof(UniformBufferObject);

   const auto swapchainImagesSize = m_swapChainImages.size();

   m_uniformBuffers.resize(swapchainImagesSize);
   m_uniformBuffersMemory.resize(swapchainImagesSize);

   for (size_t i = 0; i < swapchainImagesSize; i++)
   {
      Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                   m_uniformBuffers[i], m_uniformBuffersMemory[i]);
   }
}
void
VulkanRenderer::CreateDescriptorPool()
{
   VkDescriptorPoolSize poolSize{};
   poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   poolSize.descriptorCount = static_cast< uint32_t >(m_swapChainImages.size());

   VkDescriptorPoolCreateInfo poolInfo{};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.poolSizeCount = 1;
   poolInfo.pPoolSizes = &poolSize;
   poolInfo.maxSets = static_cast< uint32_t >(m_swapChainImages.size());

   if (vkCreateDescriptorPool(Data::vk_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
   {
      throw std::runtime_error("failed to create descriptor pool!");
   }
}

void
VulkanRenderer::CreateDescriptorSets()
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

   for (size_t i = 0; i < m_swapChainImages.size(); i++)
   {
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = m_uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);

      VkWriteDescriptorSet descriptorWrite{};
      descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet = m_descriptorSets[i];
      descriptorWrite.dstBinding = 0;
      descriptorWrite.dstArrayElement = 0;
      descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrite.descriptorCount = 1;
      descriptorWrite.pBufferInfo = &bufferInfo;

      vkUpdateDescriptorSets(Data::vk_device, 1, &descriptorWrite, 0, nullptr);
   }
}




void
VulkanRenderer::Initialize(GLFWwindow* windowHandle)
{
   CreateInstance();

   utils::Assert(glfwCreateWindowSurface(Data::vk_instance, windowHandle, nullptr, &m_surface)
                    == VK_SUCCESS,
                 "failed to create window surface!");

   CreateDevice();
   CreateSwapchain(windowHandle);
   CreateImageViews();
   CreateRenderPass();
   CreateDescriptorSetLayout();
   CreatePipeline();
   CreateFramebuffers();
   CreateCommandPoolAndBuffers();
   CreateSyncObjects();
}

void
VulkanRenderer::UpdateUniformBuffer(uint32_t currentImage)
{
   static auto startTime = std::chrono::high_resolution_clock::now();

   auto currentTime = std::chrono::high_resolution_clock::now();
   float time =
      std::chrono::duration< float, std::chrono::seconds::period >(currentTime - startTime).count();

   UniformBufferObject ubo{};
   ubo.model =
      glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
   ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                          glm::vec3(0.0f, 0.0f, 1.0f));
   ubo.proj = glm::perspective(
      glm::radians(45.0f), m_swapChainExtent.width / (float)m_swapChainExtent.height, 0.1f, 10.0f);
   ubo.proj[1][1] *= -1;

   void* data;
   vkMapMemory(Data::vk_device, m_uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
   memcpy(data, &ubo, sizeof(ubo));
   vkUnmapMemory(Data::vk_device, m_uniformBuffersMemory[currentImage]);
}

void
VulkanRenderer::Draw()
{
   vkWaitForFences(Data::vk_device, 1, &m_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

   uint32_t imageIndex;
   vkAcquireNextImageKHR(Data::vk_device, m_swapChain, UINT64_MAX,
                         m_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

   UpdateUniformBuffer(imageIndex);
   if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
   {
      vkWaitForFences(Data::vk_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
   }
   m_imagesInFlight[imageIndex] = m_inFlightFences[currentFrame];

   VkSubmitInfo submitInfo{};
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

   VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[currentFrame]};
   VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
   submitInfo.waitSemaphoreCount = 1;
   submitInfo.pWaitSemaphores = waitSemaphores;
   submitInfo.pWaitDstStageMask = waitStages;

   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

   VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[currentFrame]};
   submitInfo.signalSemaphoreCount = 1;
   submitInfo.pSignalSemaphores = signalSemaphores;

   vkResetFences(Data::vk_device, 1, &m_inFlightFences[currentFrame]);

   utils::Assert(vkQueueSubmit(Data::vk_graphicsQueue, 1, &submitInfo, m_inFlightFences[currentFrame])
                    == VK_SUCCESS,
                 "failed to submit draw command buffer!");

   VkPresentInfoKHR presentInfo{};
   presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = signalSemaphores;

   VkSwapchainKHR swapChains[] = {m_swapChain};
   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = swapChains;

   presentInfo.pImageIndices = &imageIndex;

   vkQueuePresentKHR(m_presentQueue, &presentInfo);

   currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void
VulkanRenderer::CreateInstance()
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

   const auto result = vkCreateInstance(&createInfo, nullptr, &Data::vk_instance);
   utils::Assert(result == VK_SUCCESS, "instance not created properly!");
}

void
VulkanRenderer::CreateDevice()
{
   uint32_t deviceCount = 0;
   vkEnumeratePhysicalDevices(Data::vk_instance, &deviceCount, nullptr);
   utils::Assert(deviceCount > 0, "failed to find GPUs with Vulkan support!");

   std::vector< VkPhysicalDevice > devices(deviceCount);
   vkEnumeratePhysicalDevices(Data::vk_instance, &deviceCount, devices.data());

   for (const auto& device : devices)
   {
      if (isDeviceSuitable(device, m_surface))
      {
         VkPhysicalDeviceProperties deviceProps{};
         vkGetPhysicalDeviceProperties(device, &deviceProps);
         trace::Logger::Info("Device found! Using {}", deviceProps.deviceName);

         Data::vk_physicalDevice = device;
         m_msaaSamples = getMaxUsableSampleCount(Data::vk_physicalDevice);
         break;
      }
   }

   utils::Assert(Data::vk_physicalDevice != VK_NULL_HANDLE, "failed to find a suitable GPU!");


   QueueFamilyIndices indices = findQueueFamilies(Data::vk_physicalDevice, m_surface);

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

   VkPhysicalDeviceFeatures deviceFeatures{};
   deviceFeatures.samplerAnisotropy = VK_TRUE;

   VkDeviceCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

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

   utils::Assert(vkCreateDevice(Data::vk_physicalDevice, &createInfo, nullptr, &Data::vk_device) == VK_SUCCESS,
                 "failed to create logical device!");

   vkGetDeviceQueue(Data::vk_device, indices.graphicsFamily.value(), 0, &Data::vk_graphicsQueue);
   vkGetDeviceQueue(Data::vk_device, indices.presentFamily.value(), 0, &m_presentQueue);
}

void
VulkanRenderer::CreateSwapchain(GLFWwindow* windowHandle)
{
   SwapChainSupportDetails swapChainSupport = querySwapChainSupport(Data::vk_physicalDevice, m_surface);

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
   swapChainCreateInfo.surface = m_surface;

   swapChainCreateInfo.minImageCount = imageCount;
   swapChainCreateInfo.imageFormat = surfaceFormat.format;
   swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
   swapChainCreateInfo.imageExtent = extent;
   swapChainCreateInfo.imageArrayLayers = 1;
   swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

   QueueFamilyIndices indicesSecond = findQueueFamilies(Data::vk_physicalDevice, m_surface);
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

   utils::Assert(vkCreateSwapchainKHR(Data::vk_device, &swapChainCreateInfo, nullptr, &m_swapChain)
                    == VK_SUCCESS,
                 "failed to create swap chain!");

   vkGetSwapchainImagesKHR(Data::vk_device, m_swapChain, &imageCount, nullptr);
   m_swapChainImages.resize(imageCount);
   vkGetSwapchainImagesKHR(Data::vk_device, m_swapChain, &imageCount, m_swapChainImages.data());

   m_swapChainImageFormat = surfaceFormat.format;
   m_swapChainExtent = extent;
}

void
VulkanRenderer::CreateImageViews()
{
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

      utils::Assert(
         vkCreateImageView(Data::vk_device, &imageViewCreateInfo, nullptr, &m_swapChainImageViews[i])
            == VK_SUCCESS,
         "failed to create image views!");
   }
}

void
VulkanRenderer::CreateDescriptorSetLayout()
{
   VkDescriptorSetLayoutBinding uboLayoutBinding{};
   uboLayoutBinding.binding = 0;
   uboLayoutBinding.descriptorCount = 1;
   uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   uboLayoutBinding.pImmutableSamplers = nullptr;
   uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

   VkDescriptorSetLayoutCreateInfo layoutInfo{};
   layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   layoutInfo.bindingCount = 1;
   layoutInfo.pBindings = &uboLayoutBinding;

   if (vkCreateDescriptorSetLayout(Data::vk_device, &layoutInfo, nullptr, &m_descriptorSetLayout)
       != VK_SUCCESS)
   {
      throw std::runtime_error("failed to create descriptor set layout!");
   }
}

void
VulkanRenderer::CreateRenderPass()
{
   VkAttachmentDescription colorAttachment{};
   colorAttachment.format = m_swapChainImageFormat;
   colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
   colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
   colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

   VkAttachmentReference colorAttachmentRef{};
   colorAttachmentRef.attachment = 0;
   colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

   VkSubpassDescription subpass{};
   subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.colorAttachmentCount = 1;
   subpass.pColorAttachments = &colorAttachmentRef;

   VkSubpassDependency dependency{};
   dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
   dependency.dstSubpass = 0;
   dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   dependency.srcAccessMask = 0;
   dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

   VkRenderPassCreateInfo renderPassInfo{};
   renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   renderPassInfo.attachmentCount = 1;
   renderPassInfo.pAttachments = &colorAttachment;
   renderPassInfo.subpassCount = 1;
   renderPassInfo.pSubpasses = &subpass;
   renderPassInfo.dependencyCount = 1;
   renderPassInfo.pDependencies = &dependency;

   utils::Assert(vkCreateRenderPass(Data::vk_device, &renderPassInfo, nullptr, &m_renderPass)
                    == VK_SUCCESS,
                 "failed to create render pass!");
}

void
VulkanRenderer::CreateFramebuffers()
{
   m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

   for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
   {
      VkImageView attachments[] = {m_swapChainImageViews[i]};

      VkFramebufferCreateInfo framebufferInfo{};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = m_renderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.width = m_swapChainExtent.width;
      framebufferInfo.height = m_swapChainExtent.height;
      framebufferInfo.layers = 1;

      utils::Assert(
         vkCreateFramebuffer(Data::vk_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i])
            == VK_SUCCESS,
         "failed to create framebuffer!");
   }
}

void
VulkanRenderer::CreateCommandPoolAndBuffers()
{
   /*
    *  CREATE COMMAND POOL
    */

   QueueFamilyIndices queueFamilyIndicesTwo = findQueueFamilies(Data::vk_physicalDevice, m_surface);

   VkCommandPoolCreateInfo poolInfo{};
   poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   poolInfo.queueFamilyIndex = queueFamilyIndicesTwo.graphicsFamily.value();

   utils::Assert(vkCreateCommandPool(Data::vk_device, &poolInfo, nullptr, &Data::vk_commandPool) == VK_SUCCESS,
                 "failed to create command pool!");


   auto tex = TextureLibrary::GetTexture(TextureType::DIFFUSE_MAP, "196.png");
   CreateVertexBuffer();
   CreateIndexBuffer();
   CreateUniformBuffers();
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

   utils::Assert(vkAllocateCommandBuffers(Data::vk_device, &allocInfo, m_commandBuffers.data())
                    == VK_SUCCESS,
                 "failed to allocate command buffers!");

   for (size_t i = 0; i < m_commandBuffers.size(); i++)
   {
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      utils::Assert(vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) == VK_SUCCESS,
                    "failed to begin recording command buffer!");

      VkRenderPassBeginInfo renderPassInfoTwo{};
      renderPassInfoTwo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassInfoTwo.renderPass = m_renderPass;
      renderPassInfoTwo.framebuffer = m_swapChainFramebuffers[i];
      renderPassInfoTwo.renderArea.offset = {0, 0};
      renderPassInfoTwo.renderArea.extent = m_swapChainExtent;

      VkClearValue clearColor = {0.3f, 0.5f, 0.1f, 1.0f};

      renderPassInfoTwo.clearValueCount = 1;
      renderPassInfoTwo.pClearValues = &clearColor;

      vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfoTwo, VK_SUBPASS_CONTENTS_INLINE);

      vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

      VkBuffer vertexBuffers[] = {m_vertexBuffer};
      VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);

      vkCmdBindIndexBuffer(m_commandBuffers[i], m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

      vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);

      vkCmdDrawIndexed(m_commandBuffers[i], static_cast< uint32_t >(indices.size()), 1, 0, 0, 0);

      vkCmdEndRenderPass(m_commandBuffers[i]);

      utils::Assert(vkEndCommandBuffer(m_commandBuffers[i]) == VK_SUCCESS,
                    "failed to record command buffer!");
   }
}

void
VulkanRenderer::CreateSyncObjects()
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
      utils::Assert(
         vkCreateSemaphore(Data::vk_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i])
               == VK_SUCCESS
            && vkCreateSemaphore(Data::vk_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i])
                  == VK_SUCCESS
            && vkCreateFence(Data::vk_device, &fenceInfo, nullptr, &m_inFlightFences[i]) == VK_SUCCESS,
         "failed to create synchronization objects for a frame!");
   }
}

void
VulkanRenderer::CreatePipeline()
{
   auto [vertexInfo, fragmentInfo] =
      VulkanShader::CreateShader(Data::vk_device, "default/vert.spv", "default/frag.spv");
   VkPipelineShaderStageCreateInfo shaderStages[] = {vertexInfo.shaderInfo,
                                                     fragmentInfo.shaderInfo};

   VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
   vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   auto bindingDescription = Vertexx::getBindingDescription();
   auto attributeDescriptions = Vertexx::getAttributeDescriptions();
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
   viewport.width = static_cast< float >(m_swapChainExtent.width);
   viewport.height = static_cast< float >(m_swapChainExtent.height);
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;

   VkRect2D scissor{};
   scissor.offset = {0, 0};
   scissor.extent = m_swapChainExtent;

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
   // rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
   rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
   rasterizer.depthBiasEnable = VK_FALSE;

   VkPipelineMultisampleStateCreateInfo multisampling{};
   multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   multisampling.sampleShadingEnable = VK_FALSE;
   multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

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


   utils::Assert(vkCreatePipelineLayout(Data::vk_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout)
                    == VK_SUCCESS,
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
   pipelineInfo.pColorBlendState = &colorBlending;
   pipelineInfo.layout = m_pipelineLayout;
   pipelineInfo.renderPass = m_renderPass;
   pipelineInfo.subpass = 0;
   pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;


   utils::Assert(vkCreateGraphicsPipelines(Data::vk_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                           &m_graphicsPipeline)
                    == VK_SUCCESS,
                 "failed to create graphics pipeline!");

   // Shader info can be destroyed after the pipeline is created
   vertexInfo.Destroy();
   fragmentInfo.Destroy();
}

} // namespace shady::render::vulkan
