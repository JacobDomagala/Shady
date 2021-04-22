#include "gui.hpp"
#include "utils/file_manager.hpp"
#include "render/vulkan/vulkan_common.hpp"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <fmt/format.h>
#include <imgui.h>
#include <GLFW/glfw3.h>

namespace shady::app::gui {

static ImGui_ImplVulkanH_Window g_MainWindowData;
static ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;

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

   // Create Framebuffers
   int w, h;
   glfwGetFramebufferSize(windowHandle, &w, &h);
   // SetupVulkanWindow(wd, render::vulkan::Data::m_surface, w, h);

   //ImGui_ImplVulkan_InitInfo init_info = {};
   //init_info.Instance = render::vulkan::Data::vk_instance;
   //init_info.PhysicalDevice = render::vulkan::Data::vk_physicalDevice;
   //init_info.Device = render::vulkan::Data::vk_device;
   //init_info.QueueFamily = render::vulkan::Data::vk_graphicsQueue;
   //init_info.Queue = render::vulkan::Data::vk_graphicsQueue;
   //init_info.PipelineCache = g_PipelineCache;
   //init_info.DescriptorPool = g_DescriptorPool;
   //init_info.Allocator = g_Allocator;
   //init_info.MinImageCount = g_MinImageCount;
   //init_info.ImageCount = wd->ImageCount;
   //// init_info.CheckVkResultFn = check_vk_result;
   //ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);
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
   const auto debugWindowHeight = static_cast<float>(size.y);

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


   ImGui::SetNextWindowPos({static_cast<float>(size.x) - windowWidth, 0.0f});
   ImGui::SetNextWindowSize(ImVec2(windowWidth, debugWindowHeight));
   ImGui::Begin("Debug Window");

   ImGui::SetNextTreeNodeOpen(true);
   if (ImGui::CollapsingHeader("Depth Map"))
   {
      ImGui::Image(reinterpret_cast< void* >(static_cast<size_t>(shadowMapID)),
                   {windowWidth, windowWidth});
   }

   ImGui::End();

   ImGuiIO& io = ImGui::GetIO();
   io.DisplaySize = ImVec2(static_cast< float >(windowSize.x), static_cast< float >(windowSize.y));
   ImGui::Render();
   ImDrawData* draw_data = ImGui::GetDrawData();
   const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
   //if (!is_minimized)
   //{
   //   wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
   //   wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
   //   wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
   //   wd->ClearValue.color.float32[3] = clear_color.w;
   //   FrameRender(wd, draw_data);
   //   FramePresent(wd);
   //}
}


} // namespace shady::app::gui
