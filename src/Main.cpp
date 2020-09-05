#include "app/window.hpp"
#include "SHADERS/Shader.h"
#include "Model/Model.h"
#include "DISPLAY/Camera.h"
#include "SkyBox.h"
#include "EventListener.h"
#include "Light.h"

#include <glm/gtc/type_ptr.hpp>

int
main(int, char **)
{
  int width = 1280, height = 760;
  shady::app::Window mainWindow(width, height, "DEngine");
  // mainWindow.ShowCursor(false);
  // mainWindow.WrapMouse(false);

  glm::vec3 lightPosition(-4.0f, 8.0f, -6.0f);
  Light sun(lightPosition, glm::vec3(1.0f, 1.0f, 1.0f), DIRECTIONAL_LIGHT);

  Camera camera(glm::vec3(0.0f, 2.0f, 1.0f));
  camera.SetCameraMode(FLY);


  Shader simpleProgram;
  simpleProgram.LoadShaders(
    "./SHADERS/SOURCE/NoNormal_vs.glsl", "./SHADERS/SOURCE/NoNormal_fs.glsl");

  Shader skyBoxShaders;
  skyBoxShaders.LoadShaders("./SHADERS/SOURCE/skyBox_vs.glsl", "./SHADERS/SOURCE/skyBox_fs.glsl");
  Shader shadowShaders;
  shadowShaders.LoadShaders(
    "./SHADERS/SOURCE/ShadowDepth_vs.glsl", "./SHADERS/SOURCE/ShadowDepth_fs.glsl");

  EventListener eventListener(&mainWindow, &camera, &simpleProgram);

  SkyBox sky;
  sky.LoadCubeMap("./Models/skybox");

  Model box;
  box.LoadModelFromFile(".\\Models\\Box.obj");
  box.TranslateModel(glm::vec3(0.0f, 2.0f, 0.0f));
  box.meshes[0].AddTexture("./Models/textures/154.png", textureType::DIFFUSE_MAP);
  box.meshes[0].AddTexture("./Models/textures/154_norm.png", textureType::NORMAL_MAP);
  box.meshes[0].AddTexture("./Models/textures/154s.png", textureType::SPECULAR_MAP);
  box.TranslateModel(glm::vec3(0.0f, -0.5f, -2.0f));

  Model floor;
  floor.LoadModelFromFile("./Models/Plane/plane.obj");
  floor.meshes[0].AddTexture("./Models/textures/196.png", textureType::DIFFUSE_MAP);
  floor.meshes[0].AddTexture("./Models/textures/196_norm.png", textureType::NORMAL_MAP);
  floor.meshes[0].AddTexture("./Models/textures/196s.png", textureType::SPECULAR_MAP);
  floor.TranslateModel(glm::vec3(0.0f, -2.0f, 0.0f));
  Clock clock;

  while (true) {
    eventListener.Listen();
    clock.NewFrame();

    // DRAWING SCENE TO SHADOWMAP
    // sun.StartDrawingShadows(shadowShaders.programID);
    //
    // glCullFace(GL_FRONT);
    ////nanosuit.Draw(&mainWindow, camera, &sun, shadowShaders);
    // box.Draw(&mainWindow, camera, &sun, shadowShaders);
    // floor.Draw(&mainWindow, camera, &sun, shadowShaders);
    //
    // sun.StopDrawingShadows();
    //
    // glCullFace(GL_BACK);
    // glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // NORMAL DRAWING SCENE
    simpleProgram.UseProgram();
    glUniformMatrix4fv(glGetUniformLocation(simpleProgram.programID, "lightSpaceMatrix"),
      1,
      GL_FALSE,
      glm::value_ptr(sun.shadowMatrix));
    glUniform1f(glGetUniformLocation(simpleProgram.programID, "time"), clock.time);

    // glActiveTexture(GL_TEXTURE15);
    // glBindTexture(GL_TEXTURE_2D, sun.shadowTexture.textureID);
    // glUniform1i(glGetUniformLocation(simpleProgram.programID, "depth_map"), 15);


    floor.Draw(&mainWindow, camera, &sun, simpleProgram);
    box.Draw(&mainWindow, camera, &sun, simpleProgram);
    // sky.Draw(&mainWindow, camera, skyBoxShaders);

    camera.Update(clock.deltaTime);
    mainWindow.SwapBuffers();
  }

  return 0;
}
