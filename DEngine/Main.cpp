#include "DISPLAY/Display.h"
#include "SHADERS/Shader.h"
#include "Model/Model.h"
#include"DISPLAY/Camera.h"
#include"SkyBox.h"
#include"EventListener.h"
#include"Light.h"
#include <gtc/type_ptr.hpp>

int main(int , char**)
{
	
	int width = 1280, height = 760;
	Display mainWindow(width, height, "DEngine");
	//mainWindow.ShowCursor(false);
	mainWindow.WrapMouse(false);

	glm::vec3 lightPosition(-4.0f, 8.0f, -6.0f);
	Light sun(lightPosition, glm::vec3(1.0f, 1.0f, 1.0f), DIRECTIONAL_LIGHT);
	
	Camera camera(lightPosition);
	camera.SetCameraMode(FLY);

	Shader simpleProgram;
	simpleProgram.LoadShaders("./SHADERS/SOURCE/SimpleShader.vs",
		"./SHADERS/SOURCE/SimpleShader.fs");
	Shader skyBoxShaders;
	skyBoxShaders.LoadShaders("./SHADERS/SOURCE/skyBox.vs",
		"./SHADERS/SOURCE/skyBox.fs");
	Shader shadowShaders;
	shadowShaders.LoadShaders("./SHADERS/SOURCE/ShadowDepth.vs",
		"./SHADERS/SOURCE/ShadowDepth.fs");

	EventListener eventListener(&mainWindow, &camera, &simpleProgram);


	SkyBox sky;
	sky.LoadCubeMap("./Models/skybox");
	Model box("./Models/box.obj");
	box.meshes[0].AddTexture("./Models/textures/154.png", DIFFUSE);
	box.meshes[0].AddTexture("./Models/textures/154_norm.png", NORMAL);
	box.meshes[0].AddTexture("./Models/textures/154.png", SPECULAR);
	box.TranslateModel(glm::vec3(-2.0f, 0.5f, -1.0f));
	Model floor("./Models/Plane/plane.obj");    
	floor.meshes[0].AddTexture("./Models/textures/177.png", DIFFUSE);
	//floor.meshes[0].textures[0].textureID = sun.GetShadowMapID();
	floor.meshes[0].AddTexture("./Models/textures/177_norm.png", NORMAL);
	floor.meshes[0].AddTexture("./Models/textures/177.png", SPECULAR);
	Model nanosuit("./Models/nanosuit/nanosuit.obj");
	nanosuit.ScaleModel(glm::vec3(0.2f, 0.2f, 0.2f));

	while (!mainWindow.IsClosed())
	{
		eventListener.Listen();
		
		
		sun.StartDrawingShadows(shadowShaders.GetProgramID());
		glCullFace(GL_FRONT);
		nanosuit.Draw(&mainWindow, camera, &sun, shadowShaders);
		box.Draw(&mainWindow, camera, &sun, shadowShaders);
		floor.Draw(&mainWindow, camera, &sun, shadowShaders);
		sun.StopDrawingShadows();
		glCullFace(GL_BACK);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	
		simpleProgram.UseProgram();
		glUniformMatrix4fv(glGetUniformLocation(simpleProgram.GetProgramID(), "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(sun.GetLightMatrix()));
		
		
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(GL_TEXTURE_2D, sun.GetShadowMapID());
		glUniform1i(glGetUniformLocation(simpleProgram.GetProgramID(), "depth_map"), 15);
			
		
		nanosuit.Draw(&mainWindow, camera,&sun, simpleProgram);
		floor.Draw(&mainWindow, camera, &sun, simpleProgram);
		box.Draw(&mainWindow, camera, &sun, simpleProgram);
		sky.Draw(&mainWindow, camera, skyBoxShaders);
		camera.Update();
		mainWindow.Update();
		
	}

	return 0;
}



	
	
	
