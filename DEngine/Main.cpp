#include "DISPLAY\Display.h"
#include "SHADERS\Shader.h"
#include "MESH\Mesh.h"
#include "MESH\Texture.h"


int main(int , char**)
{
	
	int width = 1024, height = 680;
	Display mainWindow(width, height, "Projekt");
	//mainWindow.ShowCursor(false);
	//mainWindow.WrapMouse(true);

	glm::vec3 lightPosition(0.0, 0.0, -10.0);
	Movement camera(&mainWindow, &lightPosition);

	camera.SetCameraMode(FLY);

	

	Shader simpleProgram;
	simpleProgram.LoadShaders("./SHADERS/SOURCE/SimpleShader.vs",
							  "./SHADERS/SOURCE/SimpleShader.fs");




	Mesh testMesh, cube;
	testMesh.LoadShape("./Models/Floor.obj");
	cube.InitCube();
	

	Texture testTexture;
	testTexture.LoadTexture("./Models/Floor.png", &simpleProgram);
	testTexture.SetTextureQuality(HIGH);
	
	
	//-----------PETLA RENDEROWANIA--------------//
	//------------------------------------------//
	while (!mainWindow.IsClosed())
	{
		
		testTexture.Use();
		testMesh.Render(&simpleProgram, &camera,&mainWindow,15.0, glm::vec3(2.0, 2.0, 2.0), 
						glm::vec3(0.0, -2.0, 0.0), 0.0, glm::vec3(1.0, 1.0, 1.0));

		cube.Render(&simpleProgram, &camera, &mainWindow, 15.0, glm::vec3(20.0, 20.0, 20.0),
			glm::vec3(5.0, 0.0, 0.0), 0.0, glm::vec3(1.0, 1.0, 1.0));
		
		camera.Update();
		mainWindow.Update();
	

	}
	testMesh.Clean();
	testTexture.CleanUp();

	return 0;
}



	
	
	
