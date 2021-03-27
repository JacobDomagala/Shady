#include "gui.hpp"
#include "render/texture.hpp"
#include "utils/file_manager.hpp"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <fmt/format.h>
#include <imgui.h>

namespace shady::render{
   extern unsigned int texColorBuffer;
}
namespace shady::app::gui {

// render::TextureHandleType HDL = 0;
// render::TextureIDType ID = 0;
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

   ImGui_ImplGlfw_InitForOpenGL(windowHandle, true);
   ImGui_ImplOpenGL3_Init("#version 410");

   // auto tex = render::TextureLibrary::GetTexture(render::TextureType::DIFFUSE_MAP,
   //                                               (utils::FileManager::TEXTURES_DIR
   //                                                  / "metal_hammered_diffuse.jpg").string());
   // HDL = tex->GetTextureHandle();
   // ID = tex->GetTextureID();
   // tex->MakeResident();
}

void
Gui::Shutdown()
{
   ImGui_ImplOpenGL3_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();
}

void
Gui::Render(const glm::ivec2& windowSize, uint32_t shadowMapID)
{
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   const auto size = windowSize;

   auto windowWidth = static_cast< float >(size.x) / 3.0f;
   const auto toolsWindowHeight = 60.0f;
   const auto levelWindowHeight = size.y - toolsWindowHeight;
   const auto gameObjectWindowHeight = static_cast<float>(size.y);
   const auto debugWindowWidth = size.x - 2 * windowWidth;
   const auto debugWindowHeight = 100;

   ImGui::SetNextWindowPos({0, 0});
   ImGui::SetNextWindowSize(ImVec2(windowWidth, toolsWindowHeight));
   ImGui::Begin("Tools");
   ImGui::PushStyleColor(ImGuiCol_Button, {0.45f, 0.0f, 0.2f, 0.8f});
   if (ImGui::Button("Play"))
   {
      // m_parent.PlayLevel();
   }

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


   ImGui::SetNextWindowPos({size.x - windowWidth, 0});
   ImGui::SetNextWindowSize(ImVec2(windowWidth, gameObjectWindowHeight));
   ImGui::Begin("Game Object");

   ImGui::SetNextTreeNodeOpen(true);
   if (ImGui::CollapsingHeader("Shader"))
   {
      // ImGui::Image(reinterpret_cast< void* >(static_cast<size_t>(::shady::render::texColorBuffer)),
      //              {windowWidth, windowWidth});
      ImGui::Image(reinterpret_cast< void* >(static_cast<size_t>(shadowMapID)),
                   {windowWidth, windowWidth});
   }

   ImGui::End();

   ImGuiIO& io = ImGui::GetIO();
   io.DisplaySize = ImVec2(static_cast< float >(windowSize.x), static_cast< float >(windowSize.y));
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


} // namespace shady::app::gui
