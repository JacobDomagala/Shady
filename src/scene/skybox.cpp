#include "skybox.hpp"

#include "render/command.hpp"
#include "render/common.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"
#include "utils/file_manager.hpp"

#include <array>

namespace shady::scene {

using namespace render;

void
Skybox::LoadCubeMap(std::string_view directory)
{
   // Positions
   std::array< float, 24 > vertices = {
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

   // Create vertex buffer with device local memory
   // ----------------------
   const auto vertex_buffer_size = vertices.size() * sizeof(float);

   m_vertexBuffer = Buffer::CreateBuffer(
      vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
   m_vertexBuffer.CopyDataWithStaging(vertices.data(), vertex_buffer_size);

   // Create index buffer with device local memory
   // ----------------------
   constexpr auto index_buffer_size = indicies.size() * sizeof(uint32_t);

   m_indexBuffer = Buffer::CreateBuffer(
      index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
   m_indexBuffer.CopyDataWithStaging(indicies.data(), index_buffer_size);

   CreateBuffers();
   CreateImageAndSampler(directory);
   CreateDescriptorSet();
   CreatePipeline();
}

void
Skybox::CreateImageAndSampler(std::string_view directory)
{
   // Load faces to memory
   // -----------------------
   constexpr auto num_faces = 6;

   auto name = std::string{directory};

   const std::unordered_map< uint32_t, std::string > textureFaces = {
      {0, name + "_right.jpg"},  {1, name + "_left.jpg"},  {2, name + "_top.jpg"},
      {3, name + "_bottom.jpg"}, {4, name + "_front.jpg"}, {5, name + "_back.jpg"}};


   // Per face data
   std::array< render::ImageData, num_faces > faces;

   // Combined bytes for all faces
   std::vector< uint8_t > combined_faces_bytes;

   for (auto [face, name] : textureFaces)
   {
      faces[face] = utils::FileManager::ReadTexture(fmt::format("skybox/{}", name));
      const auto width = faces[face].m_size.x;
      const auto height = faces[face].m_size.y;
      const auto single_face_size = 4 * width * height;

      std::copy(&faces[face].m_bytes[0], &faces[face].m_bytes[single_face_size],
                std::back_inserter(combined_faces_bytes));
   }

   // Create Image and Sampler
   // -----------------------
   const auto width = faces[0].m_size.x;
   const auto height = faces[0].m_size.y;

   std::tie(m_image, m_imageMemory) = Texture::CreateImage(
      width, height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);

   Texture::CopyBufferToCubemapImage(m_image, width, height, combined_faces_bytes.data());

   m_sampler = Texture::CreateSampler(1);
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
   buffer.view_projection =
      Data::m_camera->GetProjection() * glm::mat4(glm::mat3(Data::m_camera->GetView()));

   m_uniformBuffer.CopyData(&buffer);
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

   auto [vertexInfo, fragmentInfo] =
      Shader::CreateShader(Data::vk_device, "default/skybox.vert.spv", "default/skybox.frag.spv");
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
   viewport.width = static_cast< float >(Data::m_deferredExtent.width);
   viewport.height = static_cast< float >(Data::m_deferredExtent.height);
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;

   VkRect2D scissor{};
   scissor.offset = {0, 0};
   scissor.extent = Data::m_deferredExtent;

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
   depthStencil.depthTestEnable = VK_FALSE;
   depthStencil.depthWriteEnable = VK_FALSE;
   depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
   depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;
   depthStencil.depthBoundsTestEnable = VK_FALSE;
   depthStencil.stencilTestEnable = VK_FALSE;

   std::array< VkPipelineColorBlendAttachmentState, 3 > colorBlendAttachment{};

   colorBlendAttachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                            | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   colorBlendAttachment[0].blendEnable = VK_FALSE;

   colorBlendAttachment[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                            | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   colorBlendAttachment[1].blendEnable = VK_FALSE;

   colorBlendAttachment[2].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                            | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   colorBlendAttachment[2].blendEnable = VK_FALSE;

   VkPipelineColorBlendStateCreateInfo colorBlending{};
   colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   colorBlending.logicOpEnable = VK_FALSE;
   colorBlending.logicOp = VK_LOGIC_OP_COPY;
   colorBlending.attachmentCount = 3;
   colorBlending.pAttachments = colorBlendAttachment.data();
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
   pipelineInfo.renderPass = Data::m_deferredRenderPass;
   pipelineInfo.subpass = 0;
   pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

   VK_CHECK(vkCreateGraphicsPipelines(Data::vk_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                      &m_pipeline),
            "Skybox pipeline creation failed!");
}

void
Skybox::Draw(VkCommandBuffer commandBuffer)
{
   vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,
                           &m_descriptorSet, 0, nullptr);
   vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

   VkDeviceSize offsets[1] = {0};
   vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.m_buffer, offsets);
   vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT32);

   vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);
}

} // namespace shady::scene
