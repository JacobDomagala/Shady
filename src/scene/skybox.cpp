#include "skybox.hpp"

#include "render/vulkan/vulkan_command.hpp"
#include "render/vulkan/vulkan_common.hpp"
#include "render/vulkan/vulkan_shader.hpp"
#include "render/vulkan/vulkan_texture.hpp"
#include "utils/file_manager.hpp"

#include <array>

namespace shady::scene {

using namespace render::vulkan;

void
Skybox::LoadCubeMap(std::string_view directory)
{
   // Positions
   std::array< float, 24 > skyboxVertices = {
      -1.0f, 1.0f,  1.0f,  // vertex 0
      -1.0f, -1.0f, 1.0f,  // vertex 1
      1.0f,  -1.0f, 1.0f,  // vertex 2
      1.0f,  1.0f,  1.0f,  // vertex 3
      -1.0f, -1.0f, -1.0f, // vertex 4
      -1.0f, 1.0f,  -1.0f, // vertex 5
      1.0f,  -1.0f, -1.0f, // vertex 6
      1.0f,  1.0f,  -1.0f  // vertex 7
   };

   std::array< uint32_t, 36 > indicies = {
      3, 2, 0, 0, 2, 1, // face 1
      6, 2, 3, 7, 6, 3, // face 2
      6, 7, 4, 7, 5, 4, // face 3
      4, 0, 1, 5, 0, 4, // face 4
      7, 3, 0, 5, 7, 0, // face 5
      2, 6, 4, 1, 2, 4  // face 6
   };

   const auto vertex_buffer_size = skyboxVertices.size() * sizeof(float);
   VkBuffer vertex_stagingBuffer;
   VkDeviceMemory vertex_stagingBufferMemory;
   Buffer::CreateBuffer(vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        vertex_stagingBuffer, vertex_stagingBufferMemory);

   void* vertex_data;
   vkMapMemory(Data::vk_device, vertex_stagingBufferMemory, 0, vertex_buffer_size, 0, &vertex_data);
   memcpy(vertex_data, skyboxVertices.data(), static_cast< size_t >(vertex_buffer_size));
   vkUnmapMemory(Data::vk_device, vertex_stagingBufferMemory);

   m_vertexBuffer = Buffer::CreateBuffer(
      vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


   Buffer::CopyBuffer(vertex_stagingBuffer, m_vertexBuffer.m_buffer, vertex_buffer_size);

   const auto index_buffer_size = indicies.size() * sizeof(uint32_t);
   VkBuffer index_stagingBuffer;
   VkDeviceMemory index_stagingBufferMemory;
   Buffer::CreateBuffer(index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        index_stagingBuffer, index_stagingBufferMemory);

   void* index_data;
   vkMapMemory(Data::vk_device, index_stagingBufferMemory, 0, index_buffer_size, 0, &index_data);
   memcpy(index_data, indicies.data(), static_cast< size_t >(index_buffer_size));
   vkUnmapMemory(Data::vk_device, index_stagingBufferMemory);

   m_indexBuffer = Buffer::CreateBuffer(
      index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


   Buffer::CopyBuffer(index_stagingBuffer, m_indexBuffer.m_buffer, index_buffer_size);

   CreateBuffers();
   CreateImageAndSampler(directory);
   CreateDescriptorSet();
   CreatePipeline();
}

void
Skybox::CreateImageAndSampler(std::string_view directory)
{
   constexpr auto num_faces = 6;

   auto name = std::string{directory};

   const std::unordered_map< uint32_t, std::string > textureFaces = {
      {0, name + "_right.jpg"},  {1, name + "_left.jpg"},  {2, name + "_top.jpg"},
      {3, name + "_bottom.jpg"}, {4, name + "_front.jpg"}, {5, name + "_back.jpg"}};


   // Preload faces to know data format
   std::array< render::ImageData, num_faces > faces;

   for (auto [face, name] : textureFaces)
   {
      faces[face] = utils::FileManager::ReadTexture(fmt::format("skybox/{}", name));
   }

   const auto width = faces[0].m_size.x;
   const auto height = faces[0].m_size.y;
   const auto single_face_size = faces[0].m_channels * width * height;
   const auto total_buffer_size = num_faces * single_face_size;
   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;
   Buffer::CreateBuffer(total_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stagingBuffer, stagingBufferMemory);


   for (auto [face, name] : textureFaces)
   {
      void* data;
      vkMapMemory(Data::vk_device, stagingBufferMemory, face * single_face_size, single_face_size,
                  0, &data);
      memcpy(data, faces[face].m_bytes.get(), static_cast< size_t >(single_face_size));
      vkUnmapMemory(Data::vk_device, stagingBufferMemory);
   }

   // Create optimal tiled target image
   VkImageCreateInfo imageCreateInfo = {};
   imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
   imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
   imageCreateInfo.mipLevels = 1;
   imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
   imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
   imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
   imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   imageCreateInfo.extent = {width, height, 1};
   imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
   // Cube faces count as array layers in Vulkan
   imageCreateInfo.arrayLayers = 6;
   // This flag is required for cube map images
   imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

   VK_CHECK(vkCreateImage(Data::vk_device, &imageCreateInfo, nullptr, &m_image),
            "Skybox's image creation failed!");

   Buffer::AllocateImageMemory(m_image, m_imageMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
   vkBindImageMemory(Data::vk_device, m_image, m_imageMemory, 0);

   // Setup buffer copy regions for each face
   std::vector< VkBufferImageCopy > bufferCopyRegions;

   for (auto [face, name] : textureFaces)
   {
      const auto& textureData = faces[face];

      VkBufferImageCopy bufferCopyRegion = {};
      bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      bufferCopyRegion.imageSubresource.mipLevel = 0;
      bufferCopyRegion.imageSubresource.baseArrayLayer = face;
      bufferCopyRegion.imageSubresource.layerCount = 1;
      bufferCopyRegion.imageExtent.width = textureData.m_size.x;
      bufferCopyRegion.imageExtent.height = textureData.m_size.y;
      bufferCopyRegion.imageExtent.depth = 1;
      bufferCopyRegion.bufferOffset =
         textureData.m_size.x * textureData.m_size.y * textureData.m_channels;

      bufferCopyRegions.push_back(bufferCopyRegion);
   }

   Texture::TransitionImageLayout(m_image, VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, true);

   VkCommandBuffer commandBuffer = Command::BeginSingleTimeCommands();

   vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, m_image,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, bufferCopyRegions.size(),
                          bufferCopyRegions.data());

   Command::EndSingleTimeCommands(commandBuffer);

   Texture::TransitionImageLayout(m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, true);

   VkPhysicalDeviceProperties properties{};
   vkGetPhysicalDeviceProperties(Data::vk_physicalDevice, &properties);

   VkSamplerCreateInfo samplerInfo{};
   samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
   samplerInfo.magFilter = VK_FILTER_LINEAR;
   samplerInfo.minFilter = VK_FILTER_LINEAR;
   samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   samplerInfo.anisotropyEnable = VK_TRUE;
   samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
   samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
   samplerInfo.unnormalizedCoordinates = VK_FALSE;
   samplerInfo.compareEnable = VK_FALSE;
   samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
   samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
   samplerInfo.minLod = 0.0f;
   samplerInfo.maxLod = 0.0f;
   samplerInfo.mipLodBias = 0.0f;

   VK_CHECK(vkCreateSampler(Data::vk_device, &samplerInfo, nullptr, &m_sampler),
            "Failed to create skybox's texture sampler!");

   m_imageView = Texture::CreateImageView(m_image, VK_FORMAT_R8G8B8A8_UNORM,
                                          VK_IMAGE_ASPECT_COLOR_BIT, 1, true);
}

void
Skybox::CreateBuffers()
{
   m_uniformBuffer = Buffer::CreateBuffer(sizeof(SkyboxUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                             | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

   m_uniformBuffer.Map();

   UpdateBuffers();
}

void
Skybox::UpdateBuffers()
{
   SkyboxUBO buffer;
   buffer.projection = Data::m_camera->GetProjection();
   buffer.view_mat = Data::m_camera->GetView();
   // Cancel out translation
   buffer.view_mat[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

void
Skybox::CreateDescriptorSet()
{
   std::array< VkDescriptorPoolSize, 2 > poolSizes{};
   poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   poolSizes[0].descriptorCount = 1;
   poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   poolSizes[1].descriptorCount = 1;

   VkDescriptorPoolCreateInfo poolInfo{};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.poolSizeCount = static_cast< uint32_t >(poolSizes.size());
   poolInfo.pPoolSizes = poolSizes.data();
   poolInfo.maxSets = 1;

   VK_CHECK(vkCreateDescriptorPool(Data::vk_device, &poolInfo, nullptr, &m_descriptorPool),
            "failed to create descriptor pool!");

   VkDescriptorSetLayoutBinding uboLayoutBinding{};
   uboLayoutBinding.binding = 0;
   uboLayoutBinding.descriptorCount = 1;
   uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   uboLayoutBinding.pImmutableSamplers = nullptr;
   uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

   VkDescriptorSetLayoutBinding samplerLayoutBinding{};
   samplerLayoutBinding.binding = 1;
   samplerLayoutBinding.descriptorCount = 1;
   samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   samplerLayoutBinding.pImmutableSamplers = nullptr;
   samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

   std::array< VkDescriptorSetLayoutBinding, 2 > bindings = {uboLayoutBinding,
                                                             samplerLayoutBinding};

   VkDescriptorSetLayoutCreateInfo layoutInfo{};
   layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   layoutInfo.bindingCount = static_cast< uint32_t >(bindings.size());
   layoutInfo.pBindings = bindings.data();

   VK_CHECK(
      vkCreateDescriptorSetLayout(Data::vk_device, &layoutInfo, nullptr, &m_descriptorSetLayout),
      "Failed to create Skybox's descriptor set layout!");

   VkDescriptorSetAllocateInfo allocateInfo = {};
   allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocateInfo.pSetLayouts = &m_descriptorSetLayout;
   allocateInfo.descriptorPool = m_descriptorPool;
   allocateInfo.descriptorSetCount = 1;

   VK_CHECK(vkAllocateDescriptorSets(Data::vk_device, &allocateInfo, &m_descriptorSet),
            "Skybox's vkAllocateDescriptorSets failed!");


   VkDescriptorBufferInfo bufferInfo{};
   bufferInfo.buffer = m_uniformBuffer.m_buffer;
   bufferInfo.offset = 0;
   bufferInfo.range = sizeof(SkyboxUBO);

   std::array< VkWriteDescriptorSet, 2 > descriptorWrites{};
   descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrites[0].dstSet = m_descriptorSet;
   descriptorWrites[0].dstBinding = 0;
   descriptorWrites[0].dstArrayElement = 0;
   descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   descriptorWrites[0].descriptorCount = 1;
   descriptorWrites[0].pBufferInfo = &bufferInfo;

   VkDescriptorImageInfo descriptorImageInfo = {};
   descriptorImageInfo.sampler = m_sampler;
   descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   descriptorImageInfo.imageView = m_imageView;

   descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrites[1].dstSet = m_descriptorSet;
   descriptorWrites[1].dstBinding = 1;
   descriptorWrites[1].dstArrayElement = 0;
   descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   descriptorWrites[1].descriptorCount = 1;
   descriptorWrites[1].pImageInfo = &descriptorImageInfo;

   vkUpdateDescriptorSets(Data::vk_device, static_cast< uint32_t >(descriptorWrites.size()),
                          descriptorWrites.data(), 0, nullptr);
}

void
Skybox::CreatePipeline()
{
   VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
   pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutInfo.setLayoutCount = 1;
   pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;


   VK_CHECK(
      vkCreatePipelineLayout(Data::vk_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout),
      "Skybox pipeline layout failed!");

   //----------------------------------------------------------------------------------------//

   auto [vertexInfo, fragmentInfo] = VulkanShader::CreateShader(
      Data::vk_device, "default/skybox.vert.spv", "default/skybox.frag.spv");
   VkPipelineShaderStageCreateInfo shaderStages[] = {vertexInfo.shaderInfo,
                                                     fragmentInfo.shaderInfo};

   VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
   vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   auto bindingDescription = SkyboxVertex::getBindingDescription();
   auto attributeDescriptions = SkyboxVertex::getAttributeDescriptions();
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
   rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
   rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
   rasterizer.depthBiasEnable = VK_FALSE;

   VkPipelineMultisampleStateCreateInfo multisampling{};
   multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   multisampling.sampleShadingEnable = VK_FALSE;
   multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

   VkPipelineDepthStencilStateCreateInfo depthStencil{};
   depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
   depthStencil.depthTestEnable = VK_TRUE;
   depthStencil.depthWriteEnable = VK_TRUE;
   depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
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
                                      &m_pipeline),
            "Skybox pipeline creation failed!");
}

void
Skybox::Draw(VkCommandBuffer commandBuffer)
{
   vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
   vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,
                           &m_descriptorSet, 0, nullptr);

   VkDeviceSize offsets[1] = {0};
   vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.m_buffer, offsets);
   vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT16);

   vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);
}

} // namespace shady::scene
