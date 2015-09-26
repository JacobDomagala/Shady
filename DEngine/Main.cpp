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

	glm::vec3 lightPosition(-2.0f, 4.0f, -1.0f);
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
	Model floor("./Models/Plane/plane.obj");    
	floor.meshes[0].AddTexture("./Models/textures/179.png", DIFFUSE);
	//floor.meshes[0].textures[0].textureID = shadowMap.textureID;
	floor.meshes[0].AddTexture("./Models/textures/179_norm.png", NORMAL);
	floor.meshes[0].AddTexture("./Models/textures/179.png", SPECULAR);
	Model nanosuit("./Models/nanosuit/nanosuit.obj");
	nanosuit.ScaleModel(glm::vec3(0.2f, 0.2f, 0.2f));

	while (!mainWindow.IsClosed())
	{
		
		//mainWindow.Clear(0.2f, 0.2f, 0.2f, 1.0f);
		eventListener.Listen();

		
		sun.StartDrawingShadows(shadowShaders.GetProgramID());
		nanosuit.Draw(&mainWindow, camera, &sun,simpleProgram);
		floor.Draw(&mainWindow, camera, &sun,simpleProgram);
		sun.StopDrawingShadows();
		
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		simpleProgram.UseProgram();
		glUniformMatrix4fv(glGetUniformLocation(simpleProgram.GetProgramID(), "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(sun.GetLightMatrix()));
		
		
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(GL_TEXTURE_2D, sun.GetShadowMapID());
		glUniform1i(glGetUniformLocation(simpleProgram.GetProgramID(), "depth_map"), 15);
			
		
		nanosuit.Draw(&mainWindow, camera,&sun, simpleProgram);
		floor.Draw(&mainWindow, camera, &sun,simpleProgram);
		sky.Draw(&mainWindow, camera, skyBoxShaders);
		camera.Update();
		mainWindow.Update();
		
	}

	return 0;
}



	
	
	
