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

	EventListener eventListener(&mainWindow, &camera, &simpleProgram);

	
	Shader skyBoxShaders; 
	skyBoxShaders.LoadShaders("./SHADERS/SOURCE/skyBox.vs", 
							  "./SHADERS/SOURCE/skyBox.fs");
	Shader shadowShaders;
	shadowShaders.LoadShaders("./SHADERS/SOURCE/ShadowDepth.vs",
						      "./SHADERS/SOURCE/ShadowDepth.fs");
	

	
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
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

		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		GLfloat near_plane = 1.0f, far_plane = 7.5f;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(camera.GetLightPosition(), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lightSpaceMatrix = lightProjection * lightView;
		// - now render scene from light's point of view
		shadowShaders.UseProgram();
		glUniformMatrix4fv(glGetUniformLocation(shadowShaders.GetProgramID(), "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		nanosuit.Draw(&mainWindow, camera, shadowShaders);
		floor.Draw(&mainWindow, camera, shadowShaders);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);



		// 2. then render scene as normal with shadow mapping (using depth map)
		
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		simpleProgram.UseProgram();
		glm::mat4 projection = mainWindow.GetProjection();
		glm::mat4 view = camera.GetWorldToViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(simpleProgram.GetProgramID(), "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(simpleProgram.GetProgramID(), "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
		// Set light uniforms
		glUniform3fv(glGetUniformLocation(simpleProgram.GetProgramID(), "vLightPosition"), 1, &lightPosition[0]);
		glUniform3fv(glGetUniformLocation(simpleProgram.GetProgramID(), "vCameraPosition"), 1, &camera.GetPosition()[0]);
		glUniformMatrix4fv(glGetUniformLocation(simpleProgram.GetProgramID(), "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		
		
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glUniform1i(glGetUniformLocation(simpleProgram.GetProgramID(), "depth_map"), 15);
			
		
		nanosuit.Draw(&mainWindow, camera, simpleProgram);
		floor.Draw(&mainWindow, camera, simpleProgram);
		//sky.Draw(&mainWindow, camera, simpleProgram);
		camera.Update();
		mainWindow.Update();
		
	}

	return 0;
}



	
	
	
