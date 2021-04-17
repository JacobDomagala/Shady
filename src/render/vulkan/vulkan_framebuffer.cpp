#include "vulkan_framebuffer.hpp"
#include "vulkan_common.hpp"

#include <fmt/format.h>

namespace shady::render::vulkan {

void
Framebuffer::Create(int32_t width, int32_t height)
{
   m_width = width;
   m_height = height;

   CreateAttachments();
   SetupRenderPass();
}

void
Framebuffer::CreateAttachments()
{
   // Color attachments

   // (World space) Positions
   CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    &m_position);

   // (World space) Normals
   CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &m_normal);

   // Albedo (color)
   CreateAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &m_albedo);

   // Depth attachment

   // Find a suitable depth format
   const auto attDepthFormat = FindDepthFormat();

   CreateAttachment(attDepthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &m_depth);
}

void
Framebuffer::CreateAttachment(VkFormat format, VkImageUsageFlagBits usage,
                              FrameBufferAttachment* attachment)
{
   VkImageAspectFlags aspectMask = 0;
   VkImageLayout imageLayout;

   attachment->format = format;

   if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
   {
      aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
   }
   if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
   {
      aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
      imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
   }

   utils::Assert(aspectMask > 0, "");

   VkImageCreateInfo image = {};
   image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   image.imageType = VK_IMAGE_TYPE_2D;
   image.format = format;
   image.extent.width = m_width;
   image.extent.height = m_height;
   image.extent.depth = 1;
   image.mipLevels = 1;
   image.arrayLayers = 1;
   image.samples = VK_SAMPLE_COUNT_1_BIT;
   image.tiling = VK_IMAGE_TILING_OPTIMAL;
   image.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;

   VkMemoryAllocateInfo memAlloc = {};
   memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

   VkMemoryRequirements memReqs;

   VK_CHECK(vkCreateImage(Data::vk_device, &image, nullptr, &attachment->image), "");
   vkGetImageMemoryRequirements(Data::vk_device, attachment->image, &memReqs);
   memAlloc.allocationSize = memReqs.size;
   memAlloc.memoryTypeIndex =
      FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
   VK_CHECK(vkAllocateMemory(Data::vk_device, &memAlloc, nullptr, &attachment->mem), "");
   VK_CHECK(vkBindImageMemory(Data::vk_device, attachment->image, attachment->mem, 0), "");

   VkImageViewCreateInfo imageView = {};
   imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
   imageView.format = format;
   imageView.subresourceRange = {};
   imageView.subresourceRange.aspectMask = aspectMask;
   imageView.subresourceRange.baseMipLevel = 0;
   imageView.subresourceRange.levelCount = 1;
   imageView.subresourceRange.baseArrayLayer = 0;
   imageView.subresourceRange.layerCount = 1;
   imageView.image = attachment->image;
   VK_CHECK(vkCreateImageView(Data::vk_device, &imageView, nullptr, &attachment->view), "");
}

void
Framebuffer::SetupRenderPass()
{
   // Set up separate renderpass with references to the color and depth attachments
   std::array< VkAttachmentDescription, 4 > attachmentDescs = {};

   // Init attachment properties
   for (uint32_t i = 0; i < 4; ++i)
   {
      attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
      attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

      if (i == 3)
      {
         attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      }
      else
      {
         attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      }
   }

   // Formats
   attachmentDescs[0].format = m_position.format;
   attachmentDescs[1].format = m_normal.format;
   attachmentDescs[2].format = m_albedo.format;
   attachmentDescs[3].format = m_depth.format;

   std::vector< VkAttachmentReference > colorReferences;
   colorReferences.push_back({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
   colorReferences.push_back({1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
   colorReferences.push_back({2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});

   VkAttachmentReference depthReference = {};
   depthReference.attachment = 3;
   depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

   VkSubpassDescription subpass = {};
   subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.pColorAttachments = colorReferences.data();
   subpass.colorAttachmentCount = static_cast< uint32_t >(colorReferences.size());
   subpass.pDepthStencilAttachment = &depthReference;

   // Use subpass dependencies for attachment layout transitions
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

   VkRenderPassCreateInfo renderPassInfo = {};
   renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   renderPassInfo.pAttachments = attachmentDescs.data();
   renderPassInfo.attachmentCount = static_cast< uint32_t >(attachmentDescs.size());
   renderPassInfo.subpassCount = 1;
   renderPassInfo.pSubpasses = &subpass;
   renderPassInfo.dependencyCount = 2;
   renderPassInfo.pDependencies = dependencies.data();

   VK_CHECK(vkCreateRenderPass(Data::vk_device, &renderPassInfo, nullptr, &m_renderPass), "");

   std::array< VkImageView, 4 > attachments;
   attachments[0] = m_position.view;
   attachments[1] = m_normal.view;
   attachments[2] = m_albedo.view;
   attachments[3] = m_depth.view;

   VkFramebufferCreateInfo fbufCreateInfo = {};
   fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   fbufCreateInfo.pNext = NULL;
   fbufCreateInfo.renderPass = m_renderPass;
   fbufCreateInfo.pAttachments = attachments.data();
   fbufCreateInfo.attachmentCount = static_cast< uint32_t >(attachments.size());
   fbufCreateInfo.width = m_width;
   fbufCreateInfo.height = m_height;
   fbufCreateInfo.layers = 1;
   VK_CHECK(vkCreateFramebuffer(Data::vk_device, &fbufCreateInfo, nullptr, &m_frameBuffer), "");

   // Create sampler to sample from the color attachments
   VkSamplerCreateInfo sampler = {};
   sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
   sampler.magFilter = VK_FILTER_NEAREST;
   sampler.minFilter = VK_FILTER_NEAREST;
   sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
   sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   sampler.addressModeV = sampler.addressModeU;
   sampler.addressModeW = sampler.addressModeU;
   sampler.mipLodBias = 0.0f;
   sampler.maxAnisotropy = 1.0f;
   sampler.minLod = 0.0f;
   sampler.maxLod = 1.0f;
   sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
   VK_CHECK(vkCreateSampler(Data::vk_device, &sampler, nullptr, &m_colorSampler), "");
}

} // namespace shady::render::vulkan
