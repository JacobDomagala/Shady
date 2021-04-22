#include "defered_pipeline.hpp"
#include "scene/perspective_camera.hpp"
#include "vertex.hpp"
#include "vulkan_buffer.hpp"
#include "vulkan_common.hpp"
#include "vulkan_shader.hpp"
#include "vulkan_texture.hpp"


#include <fmt/format.h>
#include <glm/glm.hpp>

namespace shady::render::vulkan {

float timer = 0.0f;

struct Light
{
   glm::vec4 position;
   glm::vec3 color;
   float radius;
};

struct
{
   glm::mat4 projection;
   glm::mat4 model;
   glm::mat4 view;
   glm::vec4 instancePos[3];
} uboOffscreenVS;

struct
{
   Light lights[6];
   glm::vec4 viewPos;
   int debugDisplayTarget = 0;
} uboComposition;

VkDescriptorSet&
DeferedPipeline::GetDescriptorSet()
{
   return m_descriptorSet;
}

VkPipeline
DeferedPipeline::GetCompositionPipeline()
{
   return m_compositionPipeline;
}

VkPipelineLayout
DeferedPipeline::GetPipelineLayout()
{
   return m_pipelineLayout;
}

VkSemaphore&
DeferedPipeline::GetOffscreenSemaphore()
{
   return m_offscreenSemaphore;
}

// Update matrices used for the offscreen rendering of the scene
void
DeferedPipeline::UpdateUniformBufferOffscreen()
{
   uboOffscreenVS.projection = m_camera->GetProjection();
   uboOffscreenVS.view = m_camera->GetView();
   uboOffscreenVS.model = glm::mat4(1.0f);
   m_offscreenBuffer.CopyData(&uboOffscreenVS);
   // memcpy(m_offscreenBuffer.mapped, , sizeof(uboOffscreenVS));
}

VkCommandBuffer&
DeferedPipeline::GetOffscreenCmdBuffer()
{
   return m_offscreenCommandBuffer;
}

// Update lights and parameters passed to the composition shaders
void
DeferedPipeline::UpdateUniformBufferComposition()
{
   // White
   uboComposition.lights[0].position = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
   uboComposition.lights[0].color = glm::vec3(1.5f);
   uboComposition.lights[0].radius = 15.0f * 0.25f;
   // Red
   uboComposition.lights[1].position = glm::vec4(-2.0f, 0.0f, 0.0f, 0.0f);
   uboComposition.lights[1].color = glm::vec3(1.0f, 0.0f, 0.0f);
   uboComposition.lights[1].radius = 15.0f;
   // Blue
   uboComposition.lights[2].position = glm::vec4(2.0f, -1.0f, 0.0f, 0.0f);
   uboComposition.lights[2].color = glm::vec3(0.0f, 0.0f, 2.5f);
   uboComposition.lights[2].radius = 5.0f;
   // Yellow
   uboComposition.lights[3].position = glm::vec4(0.0f, -0.9f, 0.5f, 0.0f);
   uboComposition.lights[3].color = glm::vec3(1.0f, 1.0f, 0.0f);
   uboComposition.lights[3].radius = 2.0f;
   // Green
   uboComposition.lights[4].position = glm::vec4(0.0f, -0.5f, 0.0f, 0.0f);
   uboComposition.lights[4].color = glm::vec3(0.0f, 1.0f, 0.2f);
   uboComposition.lights[4].radius = 5.0f;
   // Yellow
   uboComposition.lights[5].position = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
   uboComposition.lights[5].color = glm::vec3(1.0f, 0.7f, 0.3f);
   uboComposition.lights[5].radius = 25.0f;

   uboComposition.lights[0].position.x = sin(glm::radians(360.0f * timer)) * 5.0f;
   uboComposition.lights[0].position.z = cos(glm::radians(360.0f * timer)) * 5.0f;

   uboComposition.lights[1].position.x = -4.0f + sin(glm::radians(360.0f * timer) + 45.0f) * 2.0f;
   uboComposition.lights[1].position.z = 0.0f + cos(glm::radians(360.0f * timer) + 45.0f) * 2.0f;

   uboComposition.lights[2].position.x = 4.0f + sin(glm::radians(360.0f * timer)) * 2.0f;
   uboComposition.lights[2].position.z = 0.0f + cos(glm::radians(360.0f * timer)) * 2.0f;

   uboComposition.lights[4].position.x = 0.0f + sin(glm::radians(360.0f * timer + 90.0f)) * 5.0f;
   uboComposition.lights[4].position.z = 0.0f - cos(glm::radians(360.0f * timer + 45.0f)) * 5.0f;

   uboComposition.lights[5].position.x = 0.0f + sin(glm::radians(-360.0f * timer + 135.0f)) * 10.0f;
   uboComposition.lights[5].position.z = 0.0f - cos(glm::radians(-360.0f * timer - 45.0f)) * 10.0f;

   // Current view position
   uboComposition.viewPos =
      glm::vec4(m_camera->GetPosition(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

   uboComposition.debugDisplayTarget = m_debugDisplayTarget;

   m_compositionBuffer.CopyData(&uboComposition);
   // memcpy(uniformBuffers.composition.mapped, &uboComposition, sizeof(uboComposition));
}

void
DeferedPipeline::Initialize(VkRenderPass mainRenderPass,
                            const std::vector< VkImageView >& swapChainImageViews,
                            VkPipelineCache pipelineCache)
{
   m_pipelineCache = pipelineCache;
   m_mainRenderPass = mainRenderPass;
   m_camera = std::make_unique< scene::PerspectiveCamera >(70.0f, 16.0f / 9.0f, 0.1f, 500.0f);
   PrepareOffscreenFramebuffer();
   PrepareUniformBuffers();
   SetupDescriptorSetLayout();

   PreparePipelines();
   SetupDescriptorPool();
   SetupDescriptorSet();

   BuildDeferredCommandBuffer(swapChainImageViews);
}

void
DeferedPipeline::PrepareOffscreenFramebuffer()
{
   m_offscreenFrameBuffer.Create(2048, 2048);
}

void
DeferedPipeline::PrepareUniformBuffers()
{
   // Offscreen vertex shader
   m_offscreenBuffer = Buffer::CreateBuffer(
      sizeof(uboOffscreenVS), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
   // &uniformBuffers.offscreen

   // Deferred fragment shader
   // &uniformBuffers.composition,
   m_compositionBuffer = Buffer::CreateBuffer(
      sizeof(uboComposition), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


   // Map persistent
   m_compositionBuffer.Map();
   m_offscreenBuffer.Map();
   // uniformBuffers.offscreen.map());
   // VK_CHECK(uniformBuffers.composition.map());

   // Setup instanced model positions
   uboOffscreenVS.instancePos[0] = glm::vec4(0.0f);
   uboOffscreenVS.instancePos[1] = glm::vec4(-4.0f, 0.0, -4.0f, 0.0f);
   uboOffscreenVS.instancePos[2] = glm::vec4(4.0f, 0.0, -4.0f, 0.0f);

   // Update
   UpdateUniformBufferOffscreen();
   UpdateUniformBufferComposition();
}

void
DeferedPipeline::SetupDescriptorSetLayout()
{
   // Binding 0 : Vertex shader uniform buffer (mrt.vert)
   VkDescriptorSetLayoutBinding vertexShaderUniform{};
   vertexShaderUniform.binding = 0;
   vertexShaderUniform.descriptorCount = 1;
   vertexShaderUniform.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   vertexShaderUniform.pImmutableSamplers = nullptr;
   vertexShaderUniform.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

   // Binding 1 : Per object buffer (mrt.vert)
   VkDescriptorSetLayoutBinding perInstanceBinding{};
   perInstanceBinding.binding = 1;
   perInstanceBinding.descriptorCount = 1;
   perInstanceBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
   perInstanceBinding.pImmutableSamplers = nullptr;
   perInstanceBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

   // Binding 2 : Texture sampler (mrt.frag)
   VkDescriptorSetLayoutBinding sampler{};
   sampler.binding = 2;
   sampler.descriptorCount = 1;
   sampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
   sampler.pImmutableSamplers = nullptr;
   sampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

   // Binding 3 : Texture sampler (mrt.frag)
   VkDescriptorSetLayoutBinding textures{};
   textures.binding = 3;
   textures.descriptorCount = Data::textures.size();
   textures.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
   textures.pImmutableSamplers = nullptr;
   textures.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

   // Binding 4 : Sampler Albedo (deferred.frag)
   VkDescriptorSetLayoutBinding albedoTexture{};
   albedoTexture.binding = 4;
   albedoTexture.descriptorCount = 1;
   albedoTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   albedoTexture.pImmutableSamplers = nullptr;
   albedoTexture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

   // Binding 4 : Sampler Position (deferred.frag)
   VkDescriptorSetLayoutBinding positionsTexture{};
   positionsTexture.binding = 5;
   positionsTexture.descriptorCount = 1;
   positionsTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   positionsTexture.pImmutableSamplers = nullptr;
   positionsTexture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

   // Binding 5 : Sampler Normal (deferred.frag)
   VkDescriptorSetLayoutBinding normalsTexture{};
   normalsTexture.binding = 6;
   normalsTexture.descriptorCount = 1;
   normalsTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   normalsTexture.pImmutableSamplers = nullptr;
   normalsTexture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

   // Binding 6 : Uniform (deferred.frag)
   VkDescriptorSetLayoutBinding fragmentShaderUniform{};
   fragmentShaderUniform.binding = 7;
   fragmentShaderUniform.descriptorCount = 1;
   fragmentShaderUniform.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   fragmentShaderUniform.pImmutableSamplers = nullptr;
   fragmentShaderUniform.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

   std::array< VkDescriptorSetLayoutBinding, 8 > bindings = {
      vertexShaderUniform, perInstanceBinding, sampler,        textures,
      albedoTexture,       positionsTexture,   normalsTexture, fragmentShaderUniform};

   VkDescriptorSetLayoutCreateInfo layoutInfo{};
   layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   layoutInfo.bindingCount = static_cast< uint32_t >(bindings.size());
   layoutInfo.pBindings = bindings.data();

   VK_CHECK(
      vkCreateDescriptorSetLayout(Data::vk_device, &layoutInfo, nullptr, &m_descriptorSetLayout),
      "");

   // Shared pipeline layout used by all pipelines
   VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
   pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutCreateInfo.setLayoutCount = 1;
   pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;

   VK_CHECK(vkCreatePipelineLayout(Data::vk_device, &pipelineLayoutCreateInfo, nullptr,
                                   &m_pipelineLayout),
            "");
}

void
DeferedPipeline::PreparePipelines()
{
   VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
   inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
   inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
   inputAssembly.primitiveRestartEnable = VK_FALSE;

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

   VkPipelineColorBlendAttachmentState colorBlendAttachment{};
   colorBlendAttachment.colorWriteMask = 0xf;
   colorBlendAttachment.blendEnable = VK_FALSE;

   VkPipelineColorBlendStateCreateInfo colorBlending{};
   colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   /*colorBlending.logicOpEnable = VK_FALSE;
   colorBlending.logicOp = VK_LOGIC_OP_COPY;*/
   colorBlending.attachmentCount = 1;
   colorBlending.pAttachments = &colorBlendAttachment;

   VkPipelineDepthStencilStateCreateInfo depthStencil{};
   depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
   depthStencil.depthTestEnable = VK_TRUE;
   depthStencil.depthWriteEnable = VK_TRUE;
   depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

   VkPipelineViewportStateCreateInfo viewportState{};
   viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
   viewportState.viewportCount = 1;
   viewportState.scissorCount = 1;

   VkPipelineMultisampleStateCreateInfo multisampling{};
   multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


   std::vector< VkDynamicState > dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,
                                                        VK_DYNAMIC_STATE_SCISSOR};

   VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
   pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
   pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();
   pipelineDynamicStateCreateInfo.dynamicStateCount =
      static_cast< uint32_t >(dynamicStateEnables.size());
   pipelineDynamicStateCreateInfo.flags = 0;

   std::array< VkPipelineShaderStageCreateInfo, 2 > shaderStages;
   auto [vertexInfo, fragmentInfo] = VulkanShader::CreateShader(
      Data::vk_device, "default/deferred.vert.spv", "default/deferred.frag.spv");

   shaderStages[0] = vertexInfo.shaderInfo;
   shaderStages[1] = fragmentInfo.shaderInfo;

   VkGraphicsPipelineCreateInfo pipelineInfo{};
   pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineInfo.stageCount = static_cast< uint32_t >(shaderStages.size());
   pipelineInfo.pStages = shaderStages.data();
   pipelineInfo.pInputAssemblyState = &inputAssembly;
   pipelineInfo.pViewportState = &viewportState;
   pipelineInfo.pRasterizationState = &rasterizer;
   pipelineInfo.pMultisampleState = &multisampling;
   pipelineInfo.pDepthStencilState = &depthStencil;
   pipelineInfo.pColorBlendState = &colorBlending;
   pipelineInfo.layout = m_pipelineLayout;
   pipelineInfo.renderPass = m_mainRenderPass;
   pipelineInfo.pDynamicState = &pipelineDynamicStateCreateInfo;

   pipelineInfo.subpass = 0;
   pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

   // Final fullscreen composition pass pipeline
   rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;


   // Empty vertex input state, vertices are generated by the vertex shader
   VkPipelineVertexInputStateCreateInfo emptyInputState{};
   emptyInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

   pipelineInfo.pVertexInputState = &emptyInputState;
   VK_CHECK(vkCreateGraphicsPipelines(Data::vk_device, m_pipelineCache, 1, &pipelineInfo, nullptr,
                                      &m_compositionPipeline),
            "");

   // Vertex input state from glTF model for pipeline rendering models
   VkGraphicsPipelineCreateInfo newInfo{};
   newInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

   newInfo.pInputAssemblyState = &inputAssembly;
   newInfo.pViewportState = &viewportState;
   newInfo.pRasterizationState = &rasterizer;
   newInfo.pMultisampleState = &multisampling;
   newInfo.pDepthStencilState = &depthStencil;
   newInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
   newInfo.layout = m_pipelineLayout;

   VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
   vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   auto bindingDescription = Vertex::getBindingDescription();
   auto attributeDescriptions = Vertex::getAttributeDescriptions();
   vertexInputInfo.vertexBindingDescriptionCount = 1;
   vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast< uint32_t >(attributeDescriptions.size());
   vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
   vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

   newInfo.pVertexInputState = &vertexInputInfo;
   rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;

   // Offscreen pipeline
   std::tie(vertexInfo, fragmentInfo) =
      VulkanShader::CreateShader(Data::vk_device, "default/mrt.vert.spv", "default/mrt.frag.spv");

   shaderStages[0] = vertexInfo.shaderInfo;
   shaderStages[1] = fragmentInfo.shaderInfo;

      newInfo.stageCount = static_cast< uint32_t >(shaderStages.size());
   newInfo.pStages = shaderStages.data();

   // Separate render pass
   newInfo.renderPass = m_offscreenFrameBuffer.GetRenderPass();

   // Blend attachment states required for all color attachments
   // This is important, as color write mask will otherwise be 0x0 and you
   // won't see anything rendered to the attachment
   VkPipelineColorBlendAttachmentState firstColorBlendAttachment{};
   firstColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                              | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   firstColorBlendAttachment.blendEnable = VK_FALSE;

   VkPipelineColorBlendAttachmentState secondColorBlendAttachment{};
   secondColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                               | VK_COLOR_COMPONENT_B_BIT
                                               | VK_COLOR_COMPONENT_A_BIT;
   secondColorBlendAttachment.blendEnable = VK_FALSE;

   VkPipelineColorBlendAttachmentState thirdColorBlendAttachment{};
   thirdColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                              | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   thirdColorBlendAttachment.blendEnable = VK_FALSE;

   std::array< VkPipelineColorBlendAttachmentState, 3 > blendAttachmentStates = {
      firstColorBlendAttachment, secondColorBlendAttachment, thirdColorBlendAttachment};

   VkPipelineColorBlendStateCreateInfo colorBlendingNew{};
   colorBlendingNew.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   colorBlendingNew.attachmentCount = static_cast< uint32_t >(blendAttachmentStates.size());
   colorBlendingNew.pAttachments = blendAttachmentStates.data();

   newInfo.pColorBlendState = &colorBlendingNew;

   VK_CHECK(vkCreateGraphicsPipelines(Data::vk_device, m_pipelineCache, 1, &newInfo, nullptr,
                                      &m_offscreenPipeline),
            "");
}

void
DeferedPipeline::SetupDescriptorPool()
{
   std::array< VkDescriptorPoolSize, 5 > poolSizes{};
   poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   poolSizes[0].descriptorCount = 8; // 3 * numfrabuffers in swapchain?
   poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   poolSizes[1].descriptorCount = 9; // 3 * numfrabuffers in swapchain?
   poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
   poolSizes[2].descriptorCount = 3; // 1 * numfrabuffers in swapchain?
   poolSizes[3].type = VK_DESCRIPTOR_TYPE_SAMPLER;
   poolSizes[3].descriptorCount = 3; // 1 * numfrabuffers in swapchain?
   poolSizes[4].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
   poolSizes[4].descriptorCount = 3; // 1 * numfrabuffers in swapchain?
   
   
   VkDescriptorPoolCreateInfo poolInfo{};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.poolSizeCount = static_cast< uint32_t >(poolSizes.size());
   poolInfo.pPoolSizes = poolSizes.data();
   poolInfo.maxSets = 3; // Should be 1?

   VK_CHECK(vkCreateDescriptorPool(Data::vk_device, &poolInfo, nullptr, &m_descriptorPool), "");
}

void
DeferedPipeline::SetupDescriptorSet()
{
   std::array< VkWriteDescriptorSet, 4 > descriptorWrites{};

   VkDescriptorSetAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocInfo.descriptorPool = m_descriptorPool;
   allocInfo.descriptorSetCount = 1;
   allocInfo.pSetLayouts = &m_descriptorSetLayout;

   // Image descriptors for the offscreen color attachments
   VkDescriptorImageInfo positionsImageInfo{};
   positionsImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   positionsImageInfo.imageView = m_offscreenFrameBuffer.GetPositionsImageView();
   positionsImageInfo.sampler = m_offscreenFrameBuffer.GetSampler();

   VkDescriptorImageInfo normalsImageInfo{};
   normalsImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   normalsImageInfo.imageView = m_offscreenFrameBuffer.GetNormalsImageView();
   normalsImageInfo.sampler = m_offscreenFrameBuffer.GetSampler();

   VkDescriptorImageInfo albedoImageInfo{};
   albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   albedoImageInfo.imageView = m_offscreenFrameBuffer.GetAlbedoImageView();
   albedoImageInfo.sampler = m_offscreenFrameBuffer.GetSampler();

   // Deferred composition
   VK_CHECK(vkAllocateDescriptorSets(Data::vk_device, &allocInfo, &m_descriptorSet), "");

   // Binding 1 : Position texture target
   descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrites[0].dstSet = m_descriptorSet;
   descriptorWrites[0].dstBinding = 5;
   descriptorWrites[0].dstArrayElement = 0;
   descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   descriptorWrites[0].descriptorCount = 1;
   descriptorWrites[0].pImageInfo = &positionsImageInfo;

   // Binding 2 : Normals texture target
   descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrites[1].dstSet = m_descriptorSet;
   descriptorWrites[1].dstBinding = 6;
   descriptorWrites[1].dstArrayElement = 0;
   descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   descriptorWrites[1].descriptorCount = 1;
   descriptorWrites[1].pImageInfo = &normalsImageInfo;

   // Binding 3 : Albedo texture target
   descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrites[2].dstSet = m_descriptorSet;
   descriptorWrites[2].dstBinding = 4;
   descriptorWrites[2].dstArrayElement = 0;
   descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   descriptorWrites[2].descriptorCount = 1;
   descriptorWrites[2].pImageInfo = &albedoImageInfo;

   // Binding 4 : Fragment shader uniform buffer
   descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrites[3].dstSet = m_descriptorSet;
   descriptorWrites[3].dstBinding = 7;
   descriptorWrites[3].dstArrayElement = 0;
   descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   descriptorWrites[3].descriptorCount = 1;
   descriptorWrites[3].pBufferInfo = &m_compositionBuffer.m_descriptor;

   vkUpdateDescriptorSets(Data::vk_device, static_cast< uint32_t >(descriptorWrites.size()),
                          descriptorWrites.data(), 0, nullptr);

   // Offscreen (scene)

   const auto [imageView, sampler] =
      TextureLibrary::GetTexture(TextureType::DIFFUSE_MAP, "196.png").GetImageViewAndSampler();

   VkDescriptorBufferInfo bufferInfo{};
   bufferInfo.buffer = Data::m_uniformBuffers[0];
   bufferInfo.offset = 0;
   bufferInfo.range = sizeof(UniformBufferObject);


   VkDescriptorBufferInfo instanceBufferInfo;
   instanceBufferInfo.buffer = Data::m_ssbo[0];
   instanceBufferInfo.offset = 0;
   instanceBufferInfo.range = Data::perInstance.size() * sizeof(PerInstanceBuffer);

   /*VkDescriptorImageInfo imageInfo{};
   imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   imageInfo.imageView = imageView;
   imageInfo.sampler = sampler;*/

   VkDescriptorImageInfo* descriptorImageInfos = new VkDescriptorImageInfo[Data::textures.size()];

   uint32_t texI = 0;
   for (const auto& texture : Data::texturesVec)
   {
      descriptorImageInfos[texI].sampler = nullptr;
      descriptorImageInfos[texI].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      descriptorImageInfos[texI].imageView = texture;

      ++texI;
   }


   descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrites[0].dstSet = m_descriptorSet;
   descriptorWrites[0].dstBinding = 0;
   descriptorWrites[0].dstArrayElement = 0;
   descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   descriptorWrites[0].descriptorCount = 1;
   descriptorWrites[0].pBufferInfo = &bufferInfo;

   descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrites[1].dstSet = m_descriptorSet;
   descriptorWrites[1].dstBinding = 1;
   descriptorWrites[1].dstArrayElement = 0;
   descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
   descriptorWrites[1].descriptorCount = 1;
   descriptorWrites[1].pBufferInfo = &instanceBufferInfo;

   VkDescriptorImageInfo samplerInfo = {};
   samplerInfo.sampler = sampler;

   descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrites[2].dstSet = m_descriptorSet;
   descriptorWrites[2].dstBinding = 2;
   descriptorWrites[2].dstArrayElement = 0;
   descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
   descriptorWrites[2].descriptorCount = 1;
   descriptorWrites[2].pImageInfo = &samplerInfo;

   descriptorWrites[3].pImageInfo = descriptorImageInfos;
   descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrites[3].dstSet = m_descriptorSet;
   descriptorWrites[3].dstBinding = 3;
   descriptorWrites[3].dstArrayElement = 0;
   descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
   descriptorWrites[3].descriptorCount = Data::textures.size();
   descriptorWrites[3].pImageInfo = descriptorImageInfos;

   vkUpdateDescriptorSets(Data::vk_device, static_cast< uint32_t >(descriptorWrites.size()),
                          descriptorWrites.data(), 0, nullptr);
}

void
DeferedPipeline::BuildDeferredCommandBuffer(const std::vector< VkImageView >& swapChainImageViews)
{
   if (m_offscreenCommandBuffer == VK_NULL_HANDLE)
   {
      m_commandBuffers.resize(swapChainImageViews.size());

      VkCommandBufferAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandPool = Data::vk_commandPool;
      allocInfo.commandBufferCount = static_cast< uint32_t >(m_commandBuffers.size());

      VK_CHECK(vkAllocateCommandBuffers(Data::vk_device, &allocInfo, &m_offscreenCommandBuffer),
               "");
   }

   // Create a semaphore used to synchronize offscreen rendering and usage
   VkSemaphoreCreateInfo semaphoreCreateInfo{};
   semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

   VK_CHECK(
      vkCreateSemaphore(Data::vk_device, &semaphoreCreateInfo, nullptr, &m_offscreenSemaphore), "");

   VkCommandBufferBeginInfo cmdBufInfo{};
   cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

   // Clear values for all attachments written in the fragment shader
   std::array< VkClearValue, 4 > clearValues;
   clearValues[0].color = {{1.0f, 0.0f, 0.0f, 0.0f}};
   clearValues[1].color = {{0.0f, 1.0f, 0.0f, 0.0f}};
   clearValues[2].color = {{0.0f, 0.0f, 1.0f, 0.0f}};
   clearValues[3].depthStencil = {1.0f, 0};

   VkRenderPassBeginInfo renderPassBeginInfo = {};
   renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   renderPassBeginInfo.renderPass = m_offscreenFrameBuffer.GetRenderPass();
   renderPassBeginInfo.framebuffer = m_offscreenFrameBuffer.GetFramebuffer();
   renderPassBeginInfo.renderArea.extent.width = m_offscreenFrameBuffer.GetSize().x;
   renderPassBeginInfo.renderArea.extent.height = m_offscreenFrameBuffer.GetSize().y;
   renderPassBeginInfo.clearValueCount = static_cast< uint32_t >(clearValues.size());
   renderPassBeginInfo.pClearValues = clearValues.data();

   VK_CHECK(vkBeginCommandBuffer(m_offscreenCommandBuffer, &cmdBufInfo), "");

   vkCmdBeginRenderPass(m_offscreenCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

   VkViewport viewport{};
   viewport.width = m_offscreenFrameBuffer.GetSize().x;
   viewport.height = m_offscreenFrameBuffer.GetSize().y;
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;

   vkCmdSetViewport(m_offscreenCommandBuffer, 0, 1, &viewport);

   VkRect2D scissor{};
   scissor.extent.width = m_offscreenFrameBuffer.GetSize().x;
   scissor.extent.height = m_offscreenFrameBuffer.GetSize().y;
   scissor.offset.x = 0;
   scissor.offset.y = 0;

   vkCmdSetScissor(m_offscreenCommandBuffer, 0, 1, &scissor);

   vkCmdBindPipeline(m_offscreenCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                     m_offscreenPipeline);

   
   VkDeviceSize offsets[] = {0};
   vkCmdBindVertexBuffers(m_offscreenCommandBuffer, 0, 1, &Data::m_vertexBuffer, offsets);

   vkCmdBindIndexBuffer(m_offscreenCommandBuffer, Data::m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

   vkCmdBindDescriptorSets(m_offscreenCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                           m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);

  
   vkCmdDrawIndexedIndirectCount(m_offscreenCommandBuffer, Data::m_indirectDrawsBuffer, 0,
                                 Data::m_indirectDrawsBuffer,
                                 sizeof(VkDrawIndexedIndirectCommand) * Data::m_numMeshes,
                                 Data::m_numMeshes,
                                 sizeof(VkDrawIndexedIndirectCommand));

   //    // Background
   //    vkCmdBindDescriptorSets(m_offscreenCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
   //                            m_pipelineLayout, 0, 1, &descriptorSets.floor, 0, nullptr);
   //    models.floor.draw(m_offscreenCommandBuffer);

   //    // Instanced object
   //    vkCmdBindDescriptorSets(m_offscreenCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
   //                            m_pipelineLayout, 0, 1, &descriptorSets.model, 0, nullptr);
   //    models.model.bindBuffers(m_offscreenCommandBuffer);
   //    vkCmdDrawIndexed(m_offscreenCommandBuffer, models.model.indices.count, 3, 0, 0, 0);


   vkCmdEndRenderPass(m_offscreenCommandBuffer);

   VK_CHECK(vkEndCommandBuffer(m_offscreenCommandBuffer), "");
}

void
DeferedPipeline::DrawDeferred()
{
   // swapChain.acquireNextImage
   // VulkanExampleBase::prepareFrame();

   // The scene render command buffer has to wait for the offscreen
   // rendering to be finished before we can use the framebuffer
   // color image for sampling during final rendering
   // To ensure this we use a dedicated offscreen synchronization
   // semaphore that will be signaled when offscreen renderin
   // has been finished
   // This is necessary as an implementation may start both
   // command buffers at the same time, there is no guarantee
   // that command buffers will be executed in the order they
   // have been submitted by the application

   // Offscreen rendering

   // Wait for swap chain presentation to finish
   // submitInfo.pWaitSemaphores = &semaphores.presentComplete;
   //// Signal ready with offscreen semaphore
   // submitInfo.pSignalSemaphores = &offscreenSemaphore;

   //// Submit work
   // submitInfo.commandBufferCount = 1;
   // submitInfo.pCommandBuffers = &offScreenCmdBuffer;
   // VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE), "");

   //// Scene rendering

   //// Wait for offscreen semaphore
   // submitInfo.pWaitSemaphores = &offscreenSemaphore;
   //// Signal ready with render complete semaphore
   // submitInfo.pSignalSemaphores = &semaphores.renderComplete;

   //// Submit work
   // submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
   // VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

   // swapChain.acquireNextImage
   // VulkanExampleBase::submitFrame();
}


} // namespace shady::render::vulkan
