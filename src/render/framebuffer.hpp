#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>


namespace shady::render {

/**
 * @brief Encapsulates a single frame buffer attachment
 */
struct FramebufferAttachment
{
   /**
    * @brief Returns true if the attachment has a depth component
    */
   [[nodiscard]] bool
   hasDepth() const
   {
      std::vector< VkFormat > formats = {
         VK_FORMAT_D16_UNORM,         VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT,
         VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,   VK_FORMAT_D32_SFLOAT_S8_UINT,
      };
      return std::find(formats.begin(), formats.end(), format_) != std::end(formats);
   }

   /**
    * @brief Returns true if the attachment has a stencil component
    */
   [[nodiscard]] bool
   hasStencil() const
   {
      std::vector< VkFormat > formats = {
         VK_FORMAT_S8_UINT,
         VK_FORMAT_D16_UNORM_S8_UINT,
         VK_FORMAT_D24_UNORM_S8_UINT,
         VK_FORMAT_D32_SFLOAT_S8_UINT,
      };
      return std::find(formats.begin(), formats.end(), format_) != std::end(formats);
   }

   /**
    * @brief Returns true if the attachment is a depth and/or stencil attachment
    */
   [[nodiscard]] bool
   isDepthStencil() const
   {
      return (hasDepth() || hasStencil());
   }

 public:
   VkImage image_ = {};
   VkDeviceMemory memory_ = {};
   VkImageView view_ = {};
   VkFormat format_ = {};
   VkImageSubresourceRange subresourceRange_ = {};
   VkAttachmentDescription description_ = {};
};

/**
 * @brief Describes the attributes of an attachment to be created
 */
struct AttachmentCreateInfo
{
   uint32_t width_ = {};
   uint32_t height_ = {};
   uint32_t layerCount_ = {};
   VkFormat format_ = {};
   VkImageUsageFlags usage_ = {};
   VkSampleCountFlagBits imageSampleCount_ = VK_SAMPLE_COUNT_1_BIT;
};

class Framebuffer
{
 public:
   void
   Create(int32_t width, int32_t height);

   void
   CreateShadowMap(int32_t width, int32_t height, int32_t numLights);

   [[nodiscard]] glm::ivec2
   GetSize() const;

   [[nodiscard]] VkRenderPass
   GetRenderPass() const;

   [[nodiscard]] VkFramebuffer
   GetFramebuffer() const;

   [[nodiscard]] VkImageView
   GetPositionsImageView() const;

   [[nodiscard]] VkImageView
   GetNormalsImageView() const;

   [[nodiscard]] VkImageView
   GetAlbedoImageView() const;

   [[nodiscard]] VkImageView
   GetShadowMapView() const;

   [[nodiscard]] VkSampler
   GetSampler() const;

 private:
   /**
    * Creates a default render pass setup with one sub pass
    */
   void
   CreateRenderPass();

   /**
    * Add a new attachment described by createinfo to the framebuffer's attachment list
    *
    * @param createinfo Structure that specifies the framebuffer to be constructed
    *
    * @return Index of the new attachment
    */
   uint32_t
   AddAttachment(AttachmentCreateInfo createinfo);

   void
   CreateAttachment(VkFormat format, VkImageUsageFlagBits usage, FramebufferAttachment* attachment);

   /**
    * Creates a default sampler for sampling from any of the framebuffer attachments
    * Applications are free to create their own samplers for different use cases
    *
    * @param magFilter Magnification filter for lookups
    * @param minFilter Minification filter for lookups
    * @param adressMode Addressing mode for the U,V and W coordinates
    *
    * @return VkResult for the sampler creation
    */
   static VkSampler
   CreateSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode adressMode);

 private:
   int32_t m_width = {};
   int32_t m_height = {};
   VkFramebuffer m_framebuffer = {};
   std::vector< FramebufferAttachment > m_attachments = {};
   VkRenderPass m_renderPass = {};
   VkSampler m_sampler = {};
};

} // namespace shady::render
