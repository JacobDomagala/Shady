#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace shady::render {

/**
 * @brief Encapsulates a single frame buffer attachment
 */
struct FramebufferAttachment
{
   VkImage image;
   VkDeviceMemory memory;
   VkImageView view;
   VkFormat format;
   VkImageSubresourceRange subresourceRange;
   VkAttachmentDescription description;

   /**
    * @brief Returns true if the attachment has a depth component
    */
   bool
   hasDepth()
   {
      std::vector< VkFormat > formats = {
         VK_FORMAT_D16_UNORM,         VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT,
         VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,   VK_FORMAT_D32_SFLOAT_S8_UINT,
      };
      return std::find(formats.begin(), formats.end(), format) != std::end(formats);
   }

   /**
    * @brief Returns true if the attachment has a stencil component
    */
   bool
   hasStencil()
   {
      std::vector< VkFormat > formats = {
         VK_FORMAT_S8_UINT,
         VK_FORMAT_D16_UNORM_S8_UINT,
         VK_FORMAT_D24_UNORM_S8_UINT,
         VK_FORMAT_D32_SFLOAT_S8_UINT,
      };
      return std::find(formats.begin(), formats.end(), format) != std::end(formats);
   }

   /**
    * @brief Returns true if the attachment is a depth and/or stencil attachment
    */
   bool
   isDepthStencil()
   {
      return (hasDepth() || hasStencil());
   }
};

/**
 * @brief Describes the attributes of an attachment to be created
 */
struct AttachmentCreateInfo
{
   uint32_t width, height;
   uint32_t layerCount;
   VkFormat format;
   VkImageUsageFlags usage;
   VkSampleCountFlagBits imageSampleCount = VK_SAMPLE_COUNT_1_BIT;
};

class Framebuffer
{
 public:
   void
   Create(int32_t width, int32_t height);

   void
   CreateShadowMap(int32_t width, int32_t height, int32_t numLights);

   glm::ivec2
   GetSize() const;

   VkRenderPass
   GetRenderPass() const;

   VkFramebuffer
   GetFramebuffer() const;

   VkImageView
   GetPositionsImageView() const;

   VkImageView
   GetNormalsImageView() const;

   VkImageView
   GetAlbedoImageView() const;

   VkImageView
   GetShadowMapView() const;

   VkSampler
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
   VkSampler
   CreateSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode adressMode);

 private:
   int32_t m_width = {};
   int32_t m_height = {};
   VkFramebuffer m_framebuffer = {};
   std::vector< FramebufferAttachment > m_attachments = {};
   VkRenderPass m_renderPass = {};

   VkSampler m_sampler = {};
};

} // namespace shady::render::vulkan
