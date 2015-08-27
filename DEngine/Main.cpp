#include "DISPLAY\Display.h"
#include "SHADERS\Shader.h"
#include "Model.cpp"
#include"DISPLAY\Movement.h"
#include <gtc/type_ptr.hpp>

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

	Model ourModel("D:/Studia/Projekty/C++/Resource/Assimp/assimp-3.1.1/test/models/BLEND/Suzanne_248.blend");

	
	//-----------PETLA RENDEROWANIA--------------//
	//------------------------------------------//
	while (!mainWindow.IsClosed())
	{
		
		simpleProgram.UseProgram();   // <-- Don't forget this one!
						// Transformation matrices
		glm::mat4 projection = mainWindow.GetProjection();
		glm::mat4 view = camera.GetWorldToViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(simpleProgram.GetProgramID(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(simpleProgram.GetProgramID(), "view"), 1, GL_FALSE, glm::value_ptr(view));

		// Draw the loaded model
		glm::mat4 model;
		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// It's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(glGetUniformLocation(simpleProgram.GetProgramID(), "model"), 1, GL_FALSE, glm::value_ptr(model));
	
		ourModel.Draw(simpleProgram);
		
		
		camera.Update();
		mainWindow.Update();
	

	}
	

	return 0;
}



	
	
	
