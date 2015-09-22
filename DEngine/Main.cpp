#include "DISPLAY/Display.h"
#include "SHADERS/Shader.h"
#include "Model/Model.h"
#include"DISPLAY/Camera.h"
#include"SkyBox.h"
#include"EventListener.h"
#include <gtc/type_ptr.hpp>

int main(int , char**)
{
	
	int width = 1280, height = 720;
	Display mainWindow(width, height, "DEngine");
	//mainWindow.ShowCursor(false);
	mainWindow.WrapMouse(false);

	glm::vec3 lightPosition(0.0, 12.0, 5.0);
	Camera camera(&lightPosition);
	camera.SetCameraMode(FLY);

	Shader simpleProgram;
	simpleProgram.LoadShaders("./SHADERS/SOURCE/SimpleShader.vs",
		"./SHADERS/SOURCE/SimpleShader.fs");

	EventListener eventListener(&mainWindow, &camera, &simpleProgram);
	

	
	Shader skyBoxShaders; 
	skyBoxShaders.LoadShaders("./SHADERS/SOURCE/skyBox.vs", 
							  "./SHADERS/SOURCE/skyBox.fs");
	Shader shadowShaders;
	shadowShaders.LoadShaders("./SHADERS/SOURCE/ShadowDepth.vs",
						      "./SHADERS/SOURCE/ShadowDepth.fs");
	
	Texture shadowMap;
	shadowMap.CreateTexture(1024, 1024);
	simpleProgram.UseProgram();
	glUniform1f(glGetUniformLocation(simpleProgram.GetProgramID(), "depth_map"), 20);

	SkyBox sky;
	sky.LoadCubeMap("./Models/skybox");
	Model floor("./Models/Plane/plane.obj");    
	floor.meshes[0].AddTexture("./Models/textures/179.png", DIFFUSE);
	floor.meshes[0].AddTexture("./Models/textures/179_norm.png", NORMAL);
	floor.meshes[0].AddTexture("./Models/textures/179.png", SPECULAR);
	Model nanosuit("./Models/nanosuit/nanosuit.obj");
	nanosuit.ScaleModel(glm::vec3(0.2f, 0.2f, 0.2f));
	nanosuit.RotateModel(glm::vec3(0.0f, 1.0f, 0.0f), -90.0f);
	while (!mainWindow.IsClosed())
	{
		mainWindow.Clear(0.2f, 0.2f, 0.2f, 1.0f);
		eventListener.Listen();

		mat4 lightProjectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.0f);
		mat4 lightViewMatrix = glm::lookAt(lightPosition, vec3(0.0f), vec3(1.0f));
		mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
		shadowShaders.UseProgram();
		// 1. first render to depth map
		glUniformMatrix4fv(glGetUniformLocation(shadowShaders.GetProgramID(), "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		
		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.frameBufferID);
		glClear(GL_DEPTH_BUFFER_BIT);

		nanosuit.Draw(&mainWindow, camera, shadowShaders);
		floor.Draw(&mainWindow, camera, shadowShaders);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. then render scene as normal with shadow mapping (using depth map)
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE20);
		glBindTexture(GL_TEXTURE_2D, shadowMap.textureID);

		nanosuit.Draw(&mainWindow, camera, simpleProgram);
		floor.Draw(&mainWindow, camera, simpleProgram);

		sky.Draw(&mainWindow, camera, skyBoxShaders);

		mainWindow.Update();
	
		camera.Update();
		
	}

	return 0;
}



	
	
	
