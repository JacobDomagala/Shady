#include "framebuffer.hpp"
#include "common.hpp"

#include <fmt/format.h>
#include <numeric>
#include <algorithm>

namespace shady::render {

void
Framebuffer::Create(int32_t width, int32_t height)
{
   m_width = width;
   m_height = height;

   // Four attachments (3 color, 1 depth)
   AttachmentCreateInfo attachmentInfo = {};
   attachmentInfo.width_ = static_cast< uint32_t >(width);
   attachmentInfo.height_ = static_cast< uint32_t >(height);
   attachmentInfo.layerCount_ = 1;
   attachmentInfo.usage_ = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

   // Color attachments
   // Attachment 0: (World space) Positions
   attachmentInfo.format_ = VK_FORMAT_R16G16B16A16_SFLOAT;
   AddAttachment(attachmentInfo);

   // Attachment 1: (World space) Normals
   attachmentInfo.format_ = VK_FORMAT_R16G16B16A16_SFLOAT;
   AddAttachment(attachmentInfo);

   // Attachment 2: Albedo (color)
   attachmentInfo.format_ = VK_FORMAT_R8G8B8A8_UNORM;
   AddAttachment(attachmentInfo);

   // Depth attachment
   // Find a suitable depth format
   const auto attDepthFormat = FindDepthFormat();

   attachmentInfo.format_ = attDepthFormat;
   attachmentInfo.usage_ = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
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
   attachmentInfo.format_ = SHADOWMAP_FORMAT;
   attachmentInfo.width_ = static_cast< uint32_t >(width);
   attachmentInfo.height_ = static_cast< uint32_t >(height);
   attachmentInfo.layerCount_ = static_cast< uint32_t >(numLights);
   attachmentInfo.usage_ = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

   AddAttachment(attachmentInfo);

   // Create sampler to sample from to depth attachment
   // Used to sample in the fragment shader for shadowed rendering
   m_sampler =
      CreateSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

   // Create default renderpass for the framebuffer
   CreateRenderPass();
}

glm::ivec2
Framebuffer::GetSize() const
{
   return {m_width, m_height};
}

VkRenderPass
Framebuffer::GetRenderPass() const
{
   return m_renderPass;
}

VkFramebuffer
Framebuffer::GetFramebuffer() const
{
   return m_framebuffer;
}

VkSampler
Framebuffer::GetSampler() const
{
   return m_sampler;
}

VkImageView
Framebuffer::GetPositionsImageView() const
{
   utils::Assert(not m_attachments.empty(), "");
   return m_attachments[0].view_;
}

VkImageView
Framebuffer::GetNormalsImageView() const
{
   utils::Assert(m_attachments.size() > 1, "");
   return m_attachments[1].view_;
}

VkImageView
Framebuffer::GetAlbedoImageView() const
{
   utils::Assert(m_attachments.size() > 2, "");
   return m_attachments[2].view_;
}

VkImageView
Framebuffer::GetShadowMapView() const
{
   utils::Assert(not m_attachments.empty(), "");
   return m_attachments[0].view_;
}

VkSampler
Framebuffer::CreateSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode adressMode)
{
   VkSampler sampler{};

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
   FramebufferAttachment attachment{};

   attachment.format_ = createinfo.format_;

   VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_NONE_KHR;

   // Select aspect mask and layout depending on usage

   // Color attachment
   if (createinfo.usage_ & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
   {
      aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   }

   // Depth (and/or stencil) attachment
   if (createinfo.usage_ & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
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
   image.format = createinfo.format_;
   image.extent.width = createinfo.width_;
   image.extent.height = createinfo.height_;
   image.extent.depth = 1;
   image.mipLevels = 1;
   image.arrayLayers = createinfo.layerCount_;
   image.samples = createinfo.imageSampleCount_;
   image.tiling = VK_IMAGE_TILING_OPTIMAL;
   image.usage = createinfo.usage_;

   VkMemoryAllocateInfo memAlloc = {};
   memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   VkMemoryRequirements memReqs;

   // Create image for this attachment
   VK_CHECK(vkCreateImage(Data::vk_device, &image, nullptr, &attachment.image_), "");
   vkGetImageMemoryRequirements(Data::vk_device, attachment.image_, &memReqs);
   memAlloc.allocationSize = memReqs.size;
   memAlloc.memoryTypeIndex =
      FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
   VK_CHECK(vkAllocateMemory(Data::vk_device, &memAlloc, nullptr, &attachment.memory_), "");
   VK_CHECK(vkBindImageMemory(Data::vk_device, attachment.image_, attachment.memory_, 0), "");

   attachment.subresourceRange_ = {};
   attachment.subresourceRange_.aspectMask = aspectMask;
   attachment.subresourceRange_.levelCount = 1;
   attachment.subresourceRange_.layerCount = createinfo.layerCount_;

   VkImageViewCreateInfo imageView = {};
   imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   imageView.viewType =
      (createinfo.layerCount_ == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
   imageView.format = createinfo.format_;
   imageView.subresourceRange = attachment.subresourceRange_;
   // todo: workaround for depth+stencil attachments
   imageView.subresourceRange.aspectMask =
      (attachment.hasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspectMask;
   imageView.image = attachment.image_;
   VK_CHECK(vkCreateImageView(Data::vk_device, &imageView, nullptr, &attachment.view_), "");

   // Fill attachment description
   attachment.description_ = {};
   attachment.description_.samples = createinfo.imageSampleCount_;
   attachment.description_.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   attachment.description_.storeOp = (createinfo.usage_ & VK_IMAGE_USAGE_SAMPLED_BIT)
                                       ? VK_ATTACHMENT_STORE_OP_STORE
                                       : VK_ATTACHMENT_STORE_OP_DONT_CARE;
   attachment.description_.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   attachment.description_.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   attachment.description_.format = createinfo.format_;
   attachment.description_.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   // Final layout
   // If not, final layout depends on attachment type
   if (attachment.hasDepth() || attachment.hasStencil())
   {
      attachment.description_.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
   }
   else
   {
      attachment.description_.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   }

   m_attachments.push_back(attachment);

   return static_cast< uint32_t >(m_attachments.size() - 1);
}

void
Framebuffer::CreateRenderPass()
{
   std::vector< VkAttachmentDescription > attachmentDescriptions;
   std::transform(m_attachments.begin(), m_attachments.end(),
                  std::back_inserter(attachmentDescriptions),
                  [](const auto& attachment) { return attachment.description_; });

   // Collect attachment references
   std::vector< VkAttachmentReference > colorReferences;
   VkAttachmentReference depthReference = {};
   bool hasDepth = false;
   bool hasColor = false;

   uint32_t attachmentIndex = 0;

   for (const auto& attachment : m_attachments)
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
   std::array< VkSubpassDependency, 2 > dependencies{};

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
   std::transform(m_attachments.begin(), m_attachments.end(), std::back_inserter(attachmentViews),
                  [](const auto& attachment) { return attachment.view_; });

   // Find. max number of layers across attachments
   const uint32_t maxLayers =
      std::accumulate(m_attachments.begin(), m_attachments.end(), uint32_t{0},
                      [](uint32_t curMax, const auto& right) {
                         return std::max({curMax, right.subresourceRange_.layerCount});
                      });

   VkFramebufferCreateInfo framebufferInfo = {};
   framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   framebufferInfo.renderPass = m_renderPass;
   framebufferInfo.pAttachments = attachmentViews.data();
   framebufferInfo.attachmentCount = static_cast< uint32_t >(attachmentViews.size());
   framebufferInfo.width = static_cast< uint32_t >(m_width);
   framebufferInfo.height = static_cast< uint32_t >(m_height);
   framebufferInfo.layers = maxLayers;
   VK_CHECK(vkCreateFramebuffer(Data::vk_device, &framebufferInfo, nullptr, &m_framebuffer), "");
}

} // namespace shady::render
