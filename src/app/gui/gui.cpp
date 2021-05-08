#include "gui.hpp"
#include "app//input/input_manager.hpp"
#include "render/vulkan/vulkan_common.hpp"
#include "utils/file_manager.hpp"
#include "vulkan/vulkan_buffer.hpp"
#include "vulkan/vulkan_renderer.hpp"
#include "vulkan/vulkan_shader.hpp"
#include "vulkan/vulkan_texture.hpp"

#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <imgui.h>


namespace shady::app::gui {

using namespace shady::render::vulkan;

void
Gui::Init()
{
   // Setup Dear ImGui context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   // ImGuiIO& io = ImGui::GetIO();
   // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

   // Setup Dear ImGui style
   ImGui::StyleColorsDark();

   PrepareResources();
   PreparePipeline(Data::m_pipelineCache, Data::m_renderPass);
}

void
Gui::Shutdown()
{
   ImGui::DestroyContext();
}

bool
Gui::UpdateBuffers()
{
   ImDrawData* imDrawData = ImGui::GetDrawData();
   bool updateCmdBuffers = false;

   if (!imDrawData)
   {
      return false;
   };

   // Note: Alignment is done inside buffer creation
   VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
   VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

   // Update buffers only if vertex or index count has been changed compared to current buffer size
   if ((vertexBufferSize == 0) || (indexBufferSize == 0))
   {
      return false;
   }

   // Vertex buffer
   if ((m_vertexBuffer.m_buffer == VK_NULL_HANDLE) || (m_vertexCount != imDrawData->TotalVtxCount))
   {
      if (m_vertexBuffer.m_buffer != VK_NULL_HANDLE)
      {
         m_vertexBuffer.Unmap();
         m_vertexBuffer.Destroy();
      }

      m_vertexBuffer = Buffer::CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      m_vertexCount = imDrawData->TotalVtxCount;
      m_vertexBuffer.Map();

      updateCmdBuffers = true;
   }

   // Index buffer
   VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
   if ((m_indexBuffer.m_buffer == VK_NULL_HANDLE) || (m_indexCount < imDrawData->TotalIdxCount))
   {
      if (m_indexBuffer.m_buffer != VK_NULL_HANDLE)
      {
         m_indexBuffer.Unmap();
         m_indexBuffer.Destroy();
      }

      m_indexBuffer = Buffer::CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      m_indexCount = imDrawData->TotalIdxCount;
      m_indexBuffer.Map();

      updateCmdBuffers = true;
   }

   // Upload data
   ImDrawVert* vtxDst = (ImDrawVert*)m_vertexBuffer.m_mappedMemory;
   ImDrawIdx* idxDst = (ImDrawIdx*)m_indexBuffer.m_mappedMemory;

   for (int n = 0; n < imDrawData->CmdListsCount; n++)
   {
      const ImDrawList* cmd_list = imDrawData->CmdLists[n];
      memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
      memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
      vtxDst += cmd_list->VtxBuffer.Size;
      idxDst += cmd_list->IdxBuffer.Size;
   }

   // Flush to make writes visible to GPU
   m_vertexBuffer.Flush();
   m_indexBuffer.Flush();

   return updateCmdBuffers;
}

void
Gui::UpdateUI(const glm::ivec2& windowSize)
{
   ImGuiIO& io = ImGui::GetIO();
   io.DisplaySize = ImVec2((float)(windowSize.x), (float)(windowSize.y));

   const auto mousePos = input::InputManager::GetMousePos();

   io.MousePos = ImVec2(mousePos.x, mousePos.y);
   io.MouseDown[0] = input::InputManager::CheckButtonPressed(GLFW_MOUSE_BUTTON_1);
   io.MouseDown[1] = input::InputManager::CheckButtonPressed(GLFW_MOUSE_BUTTON_2);

   ImGui::NewFrame();

   const auto size = windowSize;

   auto windowWidth = static_cast< float >(size.x) / 3.0f;
   const auto toolsWindowHeight = 60.0f;
   const auto debugWindowHeight = static_cast< float >(size.y);

   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

   ImGui::SetNextWindowPos({0, 0});
   ImGui::SetNextWindowSize(ImVec2(windowWidth, toolsWindowHeight));
   ImGui::Begin("Tools");
   ImGui::PushStyleColor(ImGuiCol_Button, {0.45f, 0.0f, 0.2f, 0.9f});

   ImGui::PopStyleColor(1);

   const char* items[] = {"Full scene", "Position", "Normal", "Albedo", "Specular", "ShadowMap"};

   const char* combo_label = items[Data::m_renderTarget]; // Label to preview before opening the combo
                                                      // (technically it could be anything)
   if (ImGui::BeginCombo("Render target", combo_label, ImGuiComboFlags_HeightSmall))
   {
      for (int n = 0; n < IM_ARRAYSIZE(items); n++)
      {
         const bool is_selected = (Data::m_renderTarget == n);
         if (ImGui::Selectable(items[n], is_selected))
            Data::m_renderTarget = n;

         // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
         if (is_selected)
            ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
   }
   ImGui::End();


   ImGui::SetNextWindowPos({static_cast< float >(size.x) - windowWidth, 0.0f});
   ImGui::SetNextWindowSize(ImVec2(windowWidth, debugWindowHeight));
   ImGui::Begin("Debug Window");
   const auto cameraPos = Data::m_camera->GetPosition();
   const auto cameraLookAt = Data::m_camera->GetLookAtVec();
   const auto lightPos = Data::m_light->GetPosition();
   ImGui::Text("Camera Position %f, %f, %f", static_cast< double >(cameraPos.x),
               static_cast< double >(cameraPos.y), static_cast< double >(cameraPos.z));
   ImGui::Text("Camera LookAt %f, %f, %f", static_cast< double >(cameraLookAt.x),
               static_cast< double >(cameraLookAt.y), static_cast< double >(cameraLookAt.z));
   ImGui::Text("Light Position %f, %f, %f", static_cast< double >(lightPos.x),
               static_cast< double >(lightPos.y), static_cast< double >(lightPos.z));

   ImGui::End();
   ImGui::PopStyleVar();
   ImGui::Render();

   if (UpdateBuffers())
   {
      VulkanRenderer::CreateCommandBufferForDeferred();
   }
}

void
Gui::Render(VkCommandBuffer commandBuffer)
{
   ImDrawData* imDrawData = ImGui::GetDrawData();
   int32_t vertexOffset = 0;
   int32_t indexOffset = 0;

   if ((!imDrawData) || (imDrawData->CmdListsCount == 0))
   {
      return;
   }

   ImGuiIO& io = ImGui::GetIO();

   vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
   vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,
                           &m_descriptorSet, 0, NULL);

   m_pushConstant.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
   m_pushConstant.translate = glm::vec2(-1.0f);
   vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                      sizeof(PushConstBlock), &m_pushConstant);

   VkDeviceSize offsets[1] = {0};
   vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_vertexBuffer.m_buffer, offsets);
   vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT16);

   for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
   {
      const ImDrawList* cmd_list = imDrawData->CmdLists[i];
      for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
      {
         const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
         VkRect2D scissorRect;
         scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
         scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
         scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
         scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
         vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
         vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
         indexOffset += pcmd->ElemCount;
      }
      vertexOffset += cmd_list->VtxBuffer.Size;
   }
}

void
Gui::PrepareResources()
{
   ImGuiIO& io = ImGui::GetIO();

   // Create font texture
   unsigned char* fontData;
   int texWidth, texHeight;

   const auto fontFilename = (utils::FileManager::FONTS_DIR / "Roboto-Medium.ttf").string();
   // const std::string filename = getAssetPath() + "Roboto-Medium.ttf";
   io.Fonts->AddFontFromFileTTF(fontFilename.c_str(), 16.0f);

   io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
   VkDeviceSize uploadSize = static_cast< VkDeviceSize >(texWidth)
                             * static_cast< VkDeviceSize >(texHeight) * 4 * sizeof(char);

   std::tie(m_fontImage, m_fontMemory) = Texture::CreateImage(
      texWidth, texHeight, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
      VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


   m_fontView =
      Texture::CreateImageView(m_fontImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);


   // Staging buffers for font data upload

   auto stagingBuffer = Buffer::CreateBuffer(uploadSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                                | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

   stagingBuffer.Map();
   memcpy(stagingBuffer.m_mappedMemory, fontData, uploadSize);
   stagingBuffer.Unmap();


   Texture::TransitionImageLayout(m_fontImage, VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

   Texture::CopyBufferToImage(m_fontImage, texWidth, texHeight, stagingBuffer.m_buffer);


   Texture::TransitionImageLayout(m_fontImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

   // Font texture Sampler
   VkSamplerCreateInfo samplerInfo = {};
   samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
   samplerInfo.magFilter = VK_FILTER_LINEAR;
   samplerInfo.minFilter = VK_FILTER_LINEAR;
   samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
   samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
   VK_CHECK(vkCreateSampler(Data::vk_device, &samplerInfo, nullptr, &m_sampler), "");

   // Descriptor pool
   VkDescriptorPoolSize descriptorPoolSize{};
   descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   descriptorPoolSize.descriptorCount = 1;
   std::vector< VkDescriptorPoolSize > poolSizes = {descriptorPoolSize};

   VkDescriptorPoolCreateInfo descriptorPoolInfo{};
   descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   descriptorPoolInfo.poolSizeCount = static_cast< uint32_t >(poolSizes.size());
   descriptorPoolInfo.pPoolSizes = poolSizes.data();
   descriptorPoolInfo.maxSets = 2;

   VK_CHECK(
      vkCreateDescriptorPool(Data::vk_device, &descriptorPoolInfo, nullptr, &m_descriptorPool), "");

   // Descriptor set layout
   VkDescriptorSetLayoutBinding setLayoutBinding{};
   setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   setLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
   setLayoutBinding.binding = 0;
   setLayoutBinding.descriptorCount = 1;

   std::vector< VkDescriptorSetLayoutBinding > setLayoutBindings = {setLayoutBinding};

   VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
   descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   descriptorSetLayoutCreateInfo.pBindings = setLayoutBindings.data();
   descriptorSetLayoutCreateInfo.bindingCount = static_cast< uint32_t >(setLayoutBindings.size());


   VK_CHECK(vkCreateDescriptorSetLayout(Data::vk_device, &descriptorSetLayoutCreateInfo, nullptr,
                                        &m_descriptorSetLayout),
            "");

   // Descriptor set
   VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
   descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
   descriptorSetAllocateInfo.pSetLayouts = &m_descriptorSetLayout;
   descriptorSetAllocateInfo.descriptorSetCount = 1;


   VK_CHECK(vkAllocateDescriptorSets(Data::vk_device, &descriptorSetAllocateInfo, &m_descriptorSet),
            "");

   VkDescriptorImageInfo descriptorImageInfo{};
   descriptorImageInfo.sampler = m_sampler;
   descriptorImageInfo.imageView = m_fontView;
   descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

   VkWriteDescriptorSet writeDescriptorSet{};
   writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   writeDescriptorSet.dstSet = m_descriptorSet;
   writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   writeDescriptorSet.dstBinding = 0;
   writeDescriptorSet.pImageInfo = &descriptorImageInfo;
   writeDescriptorSet.descriptorCount = 1;

   std::vector< VkWriteDescriptorSet > writeDescriptorSets = {writeDescriptorSet};
   vkUpdateDescriptorSets(Data::vk_device, static_cast< uint32_t >(writeDescriptorSets.size()),
                          writeDescriptorSets.data(), 0, nullptr);
}

void
Gui::PreparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass)
{
   // Pipeline layout
   // Push constants for UI rendering parameters
   VkPushConstantRange pushConstantRange{};
   pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
   pushConstantRange.offset = 0;
   pushConstantRange.size = sizeof(PushConstBlock);

   VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
   pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutCreateInfo.setLayoutCount = 1;
   pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;

   pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
   pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
   VK_CHECK(vkCreatePipelineLayout(Data::vk_device, &pipelineLayoutCreateInfo, nullptr,
                                   &m_pipelineLayout),
            "");

   // Setup graphics pipeline for UI rendering
   VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
   pipelineInputAssemblyStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
   pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
   pipelineInputAssemblyStateCreateInfo.flags = 0;
   pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

   VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
   pipelineRasterizationStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
   pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
   pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
   pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
   pipelineRasterizationStateCreateInfo.flags = 0;
   pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
   pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;


   // Enable blending
   VkPipelineColorBlendAttachmentState blendAttachmentState{};
   blendAttachmentState.blendEnable = VK_TRUE;
   blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                         | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
   blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
   blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
   blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

   VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
   pipelineColorBlendStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   pipelineColorBlendStateCreateInfo.attachmentCount = 1;
   pipelineColorBlendStateCreateInfo.pAttachments = &blendAttachmentState;

   VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
   pipelineDepthStencilStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
   pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
   pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
   pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
   pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;

   VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
   pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
   pipelineViewportStateCreateInfo.viewportCount = 1;
   pipelineViewportStateCreateInfo.scissorCount = 1;
   pipelineViewportStateCreateInfo.flags = 0;

   VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
   pipelineMultisampleStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   pipelineMultisampleStateCreateInfo.rasterizationSamples =
      VK_SAMPLE_COUNT_1_BIT; /*Data::m_msaaSamples*/
   pipelineMultisampleStateCreateInfo.flags = 0;


   std::vector< VkDynamicState > dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,
                                                        VK_DYNAMIC_STATE_SCISSOR};

   VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
   pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
   pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();
   pipelineDynamicStateCreateInfo.dynamicStateCount =
      static_cast< uint32_t >(dynamicStateEnables.size());
   pipelineDynamicStateCreateInfo.flags = 0;

   auto [vertexInfo, fragmentInfo] =
      VulkanShader::CreateShader(Data::vk_device, "default/ui.vert.spv", "default/ui.frag.spv");
   VkPipelineShaderStageCreateInfo shaderStages[] = {vertexInfo.shaderInfo,
                                                     fragmentInfo.shaderInfo};

   VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
   pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineCreateInfo.layout = m_pipelineLayout;
   pipelineCreateInfo.renderPass = renderPass;
   pipelineCreateInfo.flags = 0;
   pipelineCreateInfo.basePipelineIndex = -1;
   pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
   pipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
   pipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
   pipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
   pipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
   pipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
   pipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
   pipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
   pipelineCreateInfo.stageCount = 2;
   pipelineCreateInfo.pStages = shaderStages;
   pipelineCreateInfo.subpass = m_subpass;

   // Vertex bindings an attributes based on ImGui vertex definition
   VkVertexInputBindingDescription vInputBindDescription{};
   vInputBindDescription.binding = 0;
   vInputBindDescription.stride = sizeof(ImDrawVert);
   vInputBindDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

   std::vector< VkVertexInputBindingDescription > vertexInputBindings = {vInputBindDescription};

   VkVertexInputAttributeDescription pos{};
   pos.location = 0;
   pos.binding = 0;
   pos.format = VK_FORMAT_R32G32_SFLOAT;
   pos.offset = offsetof(ImDrawVert, pos);

   VkVertexInputAttributeDescription uv{};
   uv.location = 1;
   uv.binding = 0;
   uv.format = VK_FORMAT_R32G32_SFLOAT;
   uv.offset = offsetof(ImDrawVert, uv);

   VkVertexInputAttributeDescription color{};
   color.location = 2;
   color.binding = 0;
   color.format = VK_FORMAT_R8G8B8A8_UNORM;
   color.offset = offsetof(ImDrawVert, col);


   std::vector< VkVertexInputAttributeDescription > vertexInputAttributes = {
      pos,   // Location 0: Position
      uv,    // Location 1: UV
      color, // Location 2: Color
   };

   VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
   pipelineVertexInputStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

   pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount =
      static_cast< uint32_t >(vertexInputBindings.size());
   pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindings.data();
   pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount =
      static_cast< uint32_t >(vertexInputAttributes.size());
   pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

   pipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;

   VK_CHECK(vkCreateGraphicsPipelines(Data::vk_device, pipelineCache, 1, &pipelineCreateInfo,
                                      nullptr, &m_pipeline),
            "");
}

} // namespace shady::app::gui
