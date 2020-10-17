#include "app/input/input_manager.hpp"
#include "app/window.hpp"
#include "render/render_command.hpp"
#include "render/renderer.hpp"
#include "scene/skybox.hpp"
#include "time/timer.hpp"
#include "utils/file_manager.hpp"
#include "render/perspective_camera.hpp"

int
main(int, char**)
{
   using namespace shady::render;
   using namespace shady::utils;
   using namespace shady::app;
   using namespace shady::app::input;
   using namespace shady::scene;

   int width = 1280, height = 760;
   Window mainWindow(width, height, "DEngine");

   Renderer::Init();
   RenderCommand::SetClearColor({0.4f, 0.1f, 0.3f, 1.0f});
   InputManager::Init(mainWindow.GetWindowHandle());

   PerspectiveCamera camera(90.0f, 16.0f/9.0f, 0.1f, 10.0f);

   Skybox skyBox;
   skyBox.LoadCubeMap((FileManager::TEXTURES_DIR / "cloudy" / "bluecloud").u8string());

   while (1)
   {
      shady::app::input::InputManager::PollEvents();
      mainWindow.Clear();

      skyBox.Draw(camera);
      mainWindow.SwapBuffers();
   }
   // mainWindow.ShowCursor(false);
   // mainWindow.WrapMouse(false);

   // glm::vec3 lightPosition(-4.0f, 8.0f, -6.0f);
   // Light sun(lightPosition, glm::vec3(1.0f, 1.0f, 1.0f), DIRECTIONAL_LIGHT);

   // Camera camera(glm::vec3(0.0f, 2.0f, 1.0f));
   // camera.SetCameraMode(FLY);


   // Shader simpleProgram;
   // simpleProgram.LoadShaders("./SHADERS/SOURCE/NoNormal_vs.glsl",
   //                          "./SHADERS/SOURCE/NoNormal_fs.glsl");

   // Shader skyBoxShaders;
   // skyBoxShaders.LoadShaders("./SHADERS/SOURCE/skyBox_vs.glsl",
   // "./SHADERS/SOURCE/skyBox_fs.glsl"); Shader shadowShaders;
   // shadowShaders.LoadShaders("./SHADERS/SOURCE/ShadowDepth_vs.glsl",
   //                          "./SHADERS/SOURCE/ShadowDepth_fs.glsl");

   // EventListener eventListener(&mainWindow, &camera, &simpleProgram);

   // Skybox sky;
   // sky.LoadCubeMap("./Models/skybox");

   // Model box;
   // box.LoadModelFromFile(".\\Models\\Box.obj");
   // box.TranslateModel(glm::vec3(0.0f, 2.0f, 0.0f));
   // box.meshes[0].AddTexture("./Models/textures/154.png", textureType::DIFFUSE_MAP);
   // box.meshes[0].AddTexture("./Models/textures/154_norm.png", textureType::NORMAL_MAP);
   // box.meshes[0].AddTexture("./Models/textures/154s.png", textureType::SPECULAR_MAP);
   // box.TranslateModel(glm::vec3(0.0f, -0.5f, -2.0f));

   // Model floor;
   // floor.LoadModelFromFile("./Models/Plane/plane.obj");
   // floor.meshes[0].AddTexture("./Models/textures/196.png", textureType::DIFFUSE_MAP);
   // floor.meshes[0].AddTexture("./Models/textures/196_norm.png", textureType::NORMAL_MAP);
   // floor.meshes[0].AddTexture("./Models/textures/196s.png", textureType::SPECULAR_MAP);
   // floor.TranslateModel(glm::vec3(0.0f, -2.0f, 0.0f));
   // shady::time::Timer clock;

   // while (true)
   //{
   //   eventListener.Listen();

   //   // DRAWING SCENE TO SHADOWMAP
   //   // sun.StartDrawingShadows(shadowShaders.programID);
   //   //
   //   // glCullFace(GL_FRONT);
   //   ////nanosuit.Draw(&mainWindow, camera, &sun, shadowShaders);
   //   // box.Draw(&mainWindow, camera, &sun, shadowShaders);
   //   // floor.Draw(&mainWindow, camera, &sun, shadowShaders);
   //   //
   //   // sun.StopDrawingShadows();
   //   //
   //   // glCullFace(GL_BACK);
   //   // glViewport(0, 0, width, height);
   //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //   // NORMAL DRAWING SCENE
   //   simpleProgram.UseProgram();
   //   glUniformMatrix4fv(glGetUniformLocation(simpleProgram.programID, "lightSpaceMatrix"), 1,
   //                      GL_FALSE, glm::value_ptr(sun.shadowMatrix));
   //   // glUniform1f(glGetUniformLocation(simpleProgram.programID, "time"), clock.time);

   //   // glActiveTexture(GL_TEXTURE15);
   //   // glBindTexture(GL_TEXTURE_2D, sun.shadowTexture.textureID);
   //   // glUniform1i(glGetUniformLocation(simpleProgram.programID, "depth_map"), 15);


   //   floor.Draw(&mainWindow, camera, &sun, simpleProgram);
   //   box.Draw(&mainWindow, camera, &sun, simpleProgram);
   //   // sky.Draw(&mainWindow, camera, skyBoxShaders);

   //   camera.Update(clock.ToggleTimer().GetMilliseconds().count());
   //   mainWindow.SwapBuffers();
   //}

   return 0;
}
