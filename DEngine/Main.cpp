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

	glm::vec3 lightPosition(0.0, 12.0, 5.0);
	Camera camera(&mainWindow, &lightPosition);

	camera.SetCameraMode(FLY);

	

	Shader simpleProgram;
	simpleProgram.LoadShaders("./SHADERS/SOURCE/SimpleShader.vs",
							  "./SHADERS/SOURCE/SimpleShader.fs");

	/*simpleProgram.LoadShaders("./SHADERS/SOURCE/noTan.vs",
		"./SHADERS/SOURCE/noTan.fs");
*/
	
	Model test("./Models/Cobblestones2/CobbleStones2.obj");    

	//Model test("./Models/Cottagemodel/Snow covered CottageOBJ.obj");   
	//Model test("./Models/Nanosuit/nanosuit.obj");
	
	test.ScaleModel(glm::vec3(0.5f, 0.5f, 0.5f));
	test.TranslateModel(glm::vec3(2.0f, -4.0f, 2.0f));
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



	
	
	
