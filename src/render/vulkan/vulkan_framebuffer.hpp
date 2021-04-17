#pragma once

#include <vulkan/vulkan.h>

namespace shady::render::vulkan {

struct FrameBufferAttachment
{
   VkImage image;
   VkDeviceMemory mem;
   VkImageView view;
   VkFormat format;
};

class Framebuffer
{
 public:
   void
   Create(int32_t width, int32_t height);

   void
   CreateAttachments();

   void SetupRenderPass();

 private:
   void
   CreateAttachment(VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachment* attachment);

 private:
   int32_t m_width, m_height;
   VkFramebuffer m_frameBuffer;
   FrameBufferAttachment m_position, m_normal, m_albedo;
   FrameBufferAttachment m_depth;
   VkRenderPass m_renderPass;
   // One sampler for the frame buffer color attachments
   VkSampler m_colorSampler = {};
};

} // namespace shady::render::vulkan
