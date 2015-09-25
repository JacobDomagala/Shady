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

	glm::vec3 lightPosition(-2.0f, 4.0f, -1.0f);
	Camera camera(&lightPosition);
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

	
	Texture shadowMap;
	shadowMap.CreateTexture(2048, 2048);
	
	SkyBox sky;
	sky.LoadCubeMap("./Models/skybox");
	Model floor("./Models/Plane/plane.obj");    
	floor.meshes[0].AddTexture("./Models/textures/179.png", DIFFUSE);
	floor.meshes[0].textures[0].textureID = shadowMap.textureID;
	floor.meshes[0].AddTexture("./Models/textures/179_norm.png", NORMAL);
	floor.meshes[0].AddTexture("./Models/textures/179.png", SPECULAR);
	Model nanosuit("./Models/nanosuit/nanosuit.obj");
	nanosuit.ScaleModel(glm::vec3(0.2f, 0.2f, 0.2f));

	while (!mainWindow.IsClosed())
	{
		
		//mainWindow.Clear(0.2f, 0.2f, 0.2f, 1.0f);
		eventListener.Listen();

		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		GLfloat near_plane = 1.0f, far_plane = 7.5f;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(camera.GetLightPosition(), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lightSpaceMatrix = lightProjection * lightView;
		// - now render scene from light's point of view
		shadowShaders.UseProgram();
		glUniformMatrix4fv(glGetUniformLocation(shadowShaders.GetProgramID(), "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		glViewport(0, 0, 2048, 2048);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.frameBufferID);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);
		nanosuit.Draw(&mainWindow, camera, shadowShaders);
		
		glCullFace(GL_BACK);
		floor.Draw(&mainWindow, camera, shadowShaders);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);



		// 2. then render scene as normal with shadow mapping (using depth map)
		
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		simpleProgram.UseProgram();
		glUniformMatrix4fv(glGetUniformLocation(simpleProgram.GetProgramID(), "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		
		
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(GL_TEXTURE_2D, shadowMap.textureID);
		glUniform1i(glGetUniformLocation(simpleProgram.GetProgramID(), "depth_map"), 15);
			
		
		nanosuit.Draw(&mainWindow, camera, simpleProgram);
		floor.Draw(&mainWindow, camera, simpleProgram);
		sky.Draw(&mainWindow, camera, skyBoxShaders);
		camera.Update();
		mainWindow.Update();
		
	}

	return 0;
}



	
	
	
