#include "DISPLAY/Display.h"
#include "SHADERS/Shader.h"
#include "Model/Model.h"
#include"DISPLAY/Camera.h"
#include"SkyBox.h"
#include"KeyListener.h"

int main(int , char**)
{
	
	int width = 1024, height = 680;
	Display mainWindow(width, height, "DEngine");
	//mainWindow.ShowCursor(false);
	mainWindow.WrapMouse(false);

	glm::vec3 lightPosition(0.0, 12.0, 5.0);
	Camera camera(&mainWindow, &lightPosition);

	camera.SetCameraMode(FLY);

	KeyListener keyListener(&mainWindow, &camera);
	

	Shader simpleProgram;
	simpleProgram.LoadShaders("./SHADERS/SOURCE/SimpleShader.vs",
							  "./SHADERS/SOURCE/SimpleShader.fs");
	Shader skyBoxShaders; 
	skyBoxShaders.LoadShaders("./SHADERS/SOURCE/skyBox.vs", 
							  "./SHADERS/SOURCE/skyBox.fs");
	
	SkyBox sky;
	sky.LoadCubeMap("./Models/skybox");
	Model floor("./Models/Plane/plane.obj");    
	floor.meshes[0].AddTexture("./Models/calinou/ice1.jpg", DIFFUSE);
	floor.meshes[0].AddTexture("./Models/calinou/ice1_n.jpg", NORMAL);
	floor.meshes[0].AddTexture("./Models/calinou/ice1_s.jpg", SPECULAR);
	Model nanosuit("./Models/nanosuit/nanosuit.obj");
	nanosuit.ScaleModel(glm::vec3(0.2f, 0.2f, 0.2f));
	nanosuit.RotateModel(glm::vec3(0.0f, 1.0f, 0.0f), -90.0f);
	while (!mainWindow.IsClosed())
	{
		mainWindow.Clear(0.2f, 0.2f, 0.2f, 1.0f);
		keyListener.KeyEvent();

		
		nanosuit.Draw(&mainWindow, camera, simpleProgram);
		floor.Draw(&mainWindow, camera, simpleProgram);
		
		sky.Draw(&mainWindow, camera, skyBoxShaders);
		mainWindow.Update();
	
		camera.Update();
		
	}

	return 0;
}



	
	
	
