#include "DISPLAY/Display.h"
#include "SHADERS/Shader.h"
#include "Model/Model.h"
#include"DISPLAY/Camera.h"


int main(int , char**)
{
	
	int width = 1024, height = 680;
	Display mainWindow(width, height, "DEngine");
	//mainWindow.ShowCursor(false);
	mainWindow.WrapMouse(false);

	glm::vec3 lightPosition(0.0, 14.0, 8.0);
	Camera camera(&mainWindow, &lightPosition);

	camera.SetCameraMode(FLY);

	

	Shader simpleProgram;
	simpleProgram.LoadShaders("./SHADERS/SOURCE/SimpleShader.vs",
							  "./SHADERS/SOURCE/SimpleShader.fs");

	
	Model test("./Models/Nanosuit/nanosuit.obj");//("./Models/Cottagemodel/Snow covered CottageOBJ.obj");

	
	//-----------PETLA RENDEROWANIA--------------//
	//------------------------------------------//
	while (!mainWindow.IsClosed())
	{
		
		test.Draw(&mainWindow, camera, simpleProgram);
	
		
		camera.Update();
		mainWindow.Update();
	

	}
	

	return 0;
}



	
	
	
