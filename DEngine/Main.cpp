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
	SkyBox sky;
	sky.LoadCubeMap(".");

	Shader simpleProgram;
	simpleProgram.LoadShaders("./SHADERS/SOURCE/SimpleShader.vs",
							  "./SHADERS/SOURCE/SimpleShader.fs");

	
	Model test("./Models/Plane/plane.obj");    
	test.meshes[0].AddTexture("./Models/textures/154.png", DIFFUSE);
	test.meshes[0].AddTexture("./Models/textures/154_norm.png", NORMAL);

	
	while (!mainWindow.IsClosed())
	{

		keyListener.KeyEvent();
		test.Draw(&mainWindow, camera, simpleProgram);
		//ky.Draw(&mainWindow, camera);
		mainWindow.Update();
		mainWindow.Clear(0.2f, 0.2f, 0.2f, 0.0f);
		camera.Update();
		
	}

	return 0;
}



	
	
	
