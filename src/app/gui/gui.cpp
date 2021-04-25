#include "gui.hpp"
#include "render/vulkan/vulkan_common.hpp"
#include "utils/file_manager.hpp"
#include "vulkan/vulkan_buffer.hpp"
#include "vulkan/vulkan_texture.hpp"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <fmt/format.h>
#include <imgui.h>


namespace shady::app::gui {

static ImGui_ImplVulkanH_Window g_MainWindowData;
static ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;

using namespace shady::render::vulkan;

void
Gui::Init(GLFWwindow* windowHandle)
{
   // Setup Dear ImGui context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO();
   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

   // Setup Dear ImGui style
   ImGui::StyleColorsDark();

   ImGui_ImplGlfw_InitForVulkan(windowHandle, false);
   PrepareResources();
   PreparePipeline(Data::m_pipelineCache, Data::m_renderPass);
}

void
Gui::Shutdown()
{
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();
}

void
Gui::Render(const glm::ivec2& windowSize, uint32_t shadowMapID)
{
   ImGui_ImplVulkan_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   const auto size = windowSize;

   auto windowWidth = static_cast< float >(size.x) / 3.0f;
   const auto toolsWindowHeight = 60.0f;
   const auto debugWindowHeight = static_cast< float >(size.y);

   ImGui::SetNextWindowPos({0, 0});
   ImGui::SetNextWindowSize(ImVec2(windowWidth, toolsWindowHeight));
   ImGui::Begin("Tools");
   ImGui::PushStyleColor(ImGuiCol_Button, {0.45f, 0.0f, 0.2f, 0.9f});

   ImGui::PopStyleColor(1);
   ImGui::SameLine();
   if (ImGui::Button("Save"))
   {
   }
   ImGui::SameLine();
   if (ImGui::Button("Load"))
   {
   }
   ImGui::SameLine();
   if (ImGui::Button("Create"))
   {
   }
   ImGui::End();


   ImGui::SetNextWindowPos({static_cast< float >(size.x) - windowWidth, 0.0f});
   ImGui::SetNextWindowSize(ImVec2(windowWidth, debugWindowHeight));
   ImGui::Begin("Debug Window");

   ImGui::SetNextTreeNodeOpen(true);
   if (ImGui::CollapsingHeader("Depth Map"))
   {
      ImGui::Image(reinterpret_cast< void* >(static_cast< size_t >(shadowMapID)),
                   {windowWidth, windowWidth});
   }

   ImGui::End();

   ImGuiIO& io = ImGui::GetIO();
   io.DisplaySize = ImVec2(static_cast< float >(windowSize.x), static_cast< float >(windowSize.y));
   ImGui::Render();
   ImDrawData* draw_data = ImGui::GetDrawData();
   const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
   // if (!is_minimized)
   //{
   //   wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
   //   wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
   //   wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
   //   wd->ClearValue.color.float32[3] = clear_color.w;
   //   FrameRender(wd, draw_data);
   //   FramePresent(wd);
   //}
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
   VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

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
   //// Pipeline layout
   //// Push constants for UI rendering parameters
   //VkPushConstantRange pushConstantRange =
   //   vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstBlock), 0);
   //VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo =
   //   vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
   //pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
   //pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
   //VK_CHECK_RESULT(
   //   vkCreatePipelineLayout(Data::vk_device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

   //// Setup graphics pipeline for UI rendering
   //VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
   //   vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
   //                                                           0, VK_FALSE);

   //VkPipelineRasterizationStateCreateInfo rasterizationState =
   //   vks::initializers::pipelineRasterizationStateCreateInfo(
   //      VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

   //// Enable blending
   //VkPipelineColorBlendAttachmentState blendAttachmentState{};
   //blendAttachmentState.blendEnable = VK_TRUE;
   //blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
   //                                      | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   //blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
   //blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   //blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
   //blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   //blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
   //blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

   //VkPipelineColorBlendStateCreateInfo colorBlendState =
   //   vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

   //VkPipelineDepthStencilStateCreateInfo depthStencilState =
   //   vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE,
   //                                                          VK_COMPARE_OP_ALWAYS);

   //VkPipelineViewportStateCreateInfo viewportState =
   //   vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

   //VkPipelineMultisampleStateCreateInfo multisampleState =
   //   vks::initializers::pipelineMultisampleStateCreateInfo(rasterizationSamples);

   //std::vector< VkDynamicState > dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,
   //                                                     VK_DYNAMIC_STATE_SCISSOR};
   //VkPipelineDynamicStateCreateInfo dynamicState =
   //   vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

   //VkGraphicsPipelineCreateInfo pipelineCreateInfo =
   //   vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass);

   //pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
   //pipelineCreateInfo.pRasterizationState = &rasterizationState;
   //pipelineCreateInfo.pColorBlendState = &colorBlendState;
   //pipelineCreateInfo.pMultisampleState = &multisampleState;
   //pipelineCreateInfo.pViewportState = &viewportState;
   //pipelineCreateInfo.pDepthStencilState = &depthStencilState;
   //pipelineCreateInfo.pDynamicState = &dynamicState;
   //pipelineCreateInfo.stageCount = static_cast< uint32_t >(shaders.size());
   //pipelineCreateInfo.pStages = shaders.data();
   //pipelineCreateInfo.subpass = subpass;

   //// Vertex bindings an attributes based on ImGui vertex definition
   //std::vector< VkVertexInputBindingDescription > vertexInputBindings = {
   //   vks::initializers::vertexInputBindingDescription(0, sizeof(ImDrawVert),
   //                                                    VK_VERTEX_INPUT_RATE_VERTEX),
   //};
   //std::vector< VkVertexInputAttributeDescription > vertexInputAttributes = {
   //   vks::initializers::vertexInputAttributeDescription(
   //      0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)), // Location 0: Position
   //   vks::initializers::vertexInputAttributeDescription(
   //      0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)), // Location 1: UV
   //   vks::initializers::vertexInputAttributeDescription(
   //      0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)), // Location 0: Color
   //};
   //VkPipelineVertexInputStateCreateInfo vertexInputState =
   //   vks::initializers::pipelineVertexInputStateCreateInfo();
   //vertexInputState.vertexBindingDescriptionCount =
   //   static_cast< uint32_t >(vertexInputBindings.size());
   //vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
   //vertexInputState.vertexAttributeDescriptionCount =
   //   static_cast< uint32_t >(vertexInputAttributes.size());
   //vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

   //pipelineCreateInfo.pVertexInputState = &vertexInputState;

   //VK_CHECK_RESULT(vkCreateGraphicsPipelines(Data::vk_device, pipelineCache, 1, &pipelineCreateInfo,
   //                                          nullptr, &pipeline));
}

} // namespace shady::app::gui
