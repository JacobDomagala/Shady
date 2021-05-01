#include "vulkan_framebuffer.hpp"
#include "vulkan_common.hpp"

#include <fmt/format.h>

namespace shady::render::vulkan {

void
Framebuffer::Create(int32_t width, int32_t height)
{
   m_width = width;
   m_height = height;

   // Four attachments (3 color, 1 depth)
   AttachmentCreateInfo attachmentInfo = {};
   attachmentInfo.width = width;
   attachmentInfo.height = height;
   attachmentInfo.layerCount = 1;
   attachmentInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

   // Color attachments
   // Attachment 0: (World space) Positions
   attachmentInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
   AddAttachment(attachmentInfo);

   // Attachment 1: (World space) Normals
   attachmentInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
   AddAttachment(attachmentInfo);

   // Attachment 2: Albedo (color)
   attachmentInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
   AddAttachment(attachmentInfo);

   // Depth attachment
   // Find a suitable depth format
   const auto attDepthFormat = FindDepthFormat();

   attachmentInfo.format = attDepthFormat;
   attachmentInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
   AddAttachment(attachmentInfo);

   // Create sampler to sample from the color attachments
   m_sampler =
      CreateSampler(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

   // Create default renderpass for the framebuffer
   CreateRenderPass();
}

void
Framebuffer::CreateShadowMap(int32_t width, int32_t height, int32_t numLights)
{
   m_width = width;
   m_height = height;

   // 16 bits of depth is enough for such a small scene
   constexpr auto SHADOWMAP_FORMAT = VK_FORMAT_D32_SFLOAT_S8_UINT;

   // Create a layered depth attachment for rendering the depth maps from the lights' point of view
   // Each layer corresponds to one of the lights
   // The actual output to the separate layers is done in the geometry shader using shader
   // instancing We will pass the matrices of the lights to the GS that selects the layer by the
   // current invocation
   AttachmentCreateInfo attachmentInfo = {};
   attachmentInfo.format = SHADOWMAP_FORMAT;
   attachmentInfo.width = width;
   attachmentInfo.height = height;
   attachmentInfo.layerCount = numLights;
   attachmentInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

   AddAttachment(attachmentInfo);

   // Create sampler to sample from to depth attachment
   // Used to sample in the fragment shader for shadowed rendering
   m_sampler =
      CreateSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

   // Create default renderpass for the framebuffer
   CreateRenderPass();
}

glm::ivec2
Framebuffer::GetSize()
{
   return {m_width, m_height};
}

VkRenderPass
Framebuffer::GetRenderPass()
{
   return m_renderPass;
}

VkFramebuffer
Framebuffer::GetFramebuffer()
{
   return m_framebuffer;
}

VkSampler
Framebuffer::GetSampler()
{
   return m_sampler;
}

VkImageView
Framebuffer::GetPositionsImageView()
{
   utils::Assert(m_attachments.size() > 0, "");
   return m_attachments[0].view;
}

VkImageView
Framebuffer::GetNormalsImageView()
{
   utils::Assert(m_attachments.size() > 1, "");
   return m_attachments[1].view;
}

VkImageView
Framebuffer::GetAlbedoImageView()
{
   utils::Assert(m_attachments.size() > 2, "");
   return m_attachments[2].view;
}

VkSampler
Framebuffer::CreateSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode adressMode)
{
   VkSampler sampler;

   VkSamplerCreateInfo samplerInfo{};
   samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
   samplerInfo.magFilter = magFilter;
   samplerInfo.minFilter = minFilter;
   samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
   samplerInfo.addressModeU = adressMode;
   samplerInfo.addressModeV = adressMode;
   samplerInfo.addressModeW = adressMode;
   samplerInfo.mipLodBias = 0.0f;
   samplerInfo.maxAnisotropy = 1.0f;
   samplerInfo.minLod = 0.0f;
   samplerInfo.maxLod = 1.0f;
   samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

   VK_CHECK(vkCreateSampler(Data::vk_device, &samplerInfo, nullptr, &sampler), "");

   return sampler;
}

uint32_t
Framebuffer::AddAttachment(AttachmentCreateInfo createinfo)
{
   FramebufferAttachment attachment;

   attachment.format = createinfo.format;

   VkImageAspectFlags aspectMask = 0;

   // Select aspect mask and layout depending on usage

   // Color attachment
   if (createinfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
   {
      aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   }

   // Depth (and/or stencil) attachment
   if (createinfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
   {
      if (attachment.hasDepth())
      {
         aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      }
      if (attachment.hasStencil())
      {
         aspectMask = aspectMask | VK_IMAGE_ASPECT_STENCIL_BIT;
      }
   }

   assert(aspectMask > 0);

   VkImageCreateInfo image = {};
   image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   image.imageType = VK_IMAGE_TYPE_2D;
   image.format = createinfo.format;
   image.extent.width = createinfo.width;
   image.extent.height = createinfo.height;
   image.extent.depth = 1;
   image.mipLevels = 1;
   image.arrayLayers = createinfo.layerCount;
   image.samples = createinfo.imageSampleCount;
   image.tiling = VK_IMAGE_TILING_OPTIMAL;
   image.usage = createinfo.usage;

   VkMemoryAllocateInfo memAlloc = {};
   memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   VkMemoryRequirements memReqs;

   // Create image for this attachment
   VK_CHECK(vkCreateImage(Data::vk_device, &image, nullptr, &attachment.image), "");
   vkGetImageMemoryRequirements(Data::vk_device, attachment.image, &memReqs);
   memAlloc.allocationSize = memReqs.size;
   memAlloc.memoryTypeIndex =
      FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
   VK_CHECK(vkAllocateMemory(Data::vk_device, &memAlloc, nullptr, &attachment.memory), "");
   VK_CHECK(vkBindImageMemory(Data::vk_device, attachment.image, attachment.memory, 0), "");

   attachment.subresourceRange = {};
   attachment.subresourceRange.aspectMask = aspectMask;
   attachment.subresourceRange.levelCount = 1;
   attachment.subresourceRange.layerCount = createinfo.layerCount;

   VkImageViewCreateInfo imageView = {};
   imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   imageView.viewType =
      (createinfo.layerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
   imageView.format = createinfo.format;
   imageView.subresourceRange = attachment.subresourceRange;
   // todo: workaround for depth+stencil attachments
   imageView.subresourceRange.aspectMask =
      (attachment.hasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspectMask;
   imageView.image = attachment.image;
   VK_CHECK(vkCreateImageView(Data::vk_device, &imageView, nullptr, &attachment.view), "");

   // Fill attachment description
   attachment.description = {};
   attachment.description.samples = createinfo.imageSampleCount;
   attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   attachment.description.storeOp = (createinfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT)
                                       ? VK_ATTACHMENT_STORE_OP_STORE
                                       : VK_ATTACHMENT_STORE_OP_DONT_CARE;
   attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   attachment.description.format = createinfo.format;
   attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   // Final layout
   // If not, final layout depends on attachment type
   if (attachment.hasDepth() || attachment.hasStencil())
   {
      attachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
   }
   else
   {
      attachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   }

   m_attachments.push_back(attachment);

   return static_cast< uint32_t >(m_attachments.size() - 1);
}

void
Framebuffer::CreateRenderPass()
{
   std::vector< VkAttachmentDescription > attachmentDescriptions;
   for (auto& attachment : m_attachments)
   {
      attachmentDescriptions.push_back(attachment.description);
   };

   // Collect attachment references
   std::vector< VkAttachmentReference > colorReferences;
   VkAttachmentReference depthReference = {};
   bool hasDepth = false;
   bool hasColor = false;

   uint32_t attachmentIndex = 0;

   for (auto& attachment : m_attachments)
   {
      if (attachment.isDepthStencil())
      {
         // Only one depth attachment allowed
         assert(!hasDepth);
         depthReference.attachment = attachmentIndex;
         depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
         hasDepth = true;
      }
      else
      {
         colorReferences.push_back({attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
         hasColor = true;
      }
      attachmentIndex++;
   };

   // Default render pass setup uses only one subpass
   VkSubpassDescription subpass = {};
   subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   if (hasColor)
   {
      subpass.pColorAttachments = colorReferences.data();
      subpass.colorAttachmentCount = static_cast< uint32_t >(colorReferences.size());
   }
   if (hasDepth)
   {
      subpass.pDepthStencilAttachment = &depthReference;
   }

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

   // Create render pass
   VkRenderPassCreateInfo renderPassInfo = {};
   renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   renderPassInfo.pAttachments = attachmentDescriptions.data();
   renderPassInfo.attachmentCount = static_cast< uint32_t >(attachmentDescriptions.size());
   renderPassInfo.subpassCount = 1;
   renderPassInfo.pSubpasses = &subpass;
   renderPassInfo.dependencyCount = 2;
   renderPassInfo.pDependencies = dependencies.data();
   VK_CHECK(vkCreateRenderPass(Data::vk_device, &renderPassInfo, nullptr, &m_renderPass), "");

   std::vector< VkImageView > attachmentViews;
   for (auto attachment : m_attachments)
   {
      attachmentViews.push_back(attachment.view);
   }

   // Find. max number of layers across attachments
   uint32_t maxLayers = 0;
   for (const auto& attachment : m_attachments)
   {
      if (attachment.subresourceRange.layerCount > maxLayers)
      {
         maxLayers = attachment.subresourceRange.layerCount;
      }
   }

   VkFramebufferCreateInfo framebufferInfo = {};
   framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   framebufferInfo.renderPass = m_renderPass;
   framebufferInfo.pAttachments = attachmentViews.data();
   framebufferInfo.attachmentCount = static_cast< uint32_t >(attachmentViews.size());
   framebufferInfo.width = m_width;
   framebufferInfo.height = m_height;
   framebufferInfo.layers = maxLayers;
   VK_CHECK(vkCreateFramebuffer(Data::vk_device, &framebufferInfo, nullptr, &m_framebuffer), "");
}

} // namespace shady::render::vulkan
