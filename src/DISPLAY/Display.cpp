#include "Display.h"

Display::Display(int width, int height, const std::string& title)
{	
	//this->height = height;
	//this->width = width;
	//aspectRatio = (float)width / height;
	//fov = 45.0;
	//nClip = 0.01f;
	//fClip = 100.0f;
	//SDL_Init(SDL_INIT_EVERYTHING);
	//projectionMatrix = glm::perspective(fov,aspectRatio, nClip, fClip);
	////projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
	//SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	//SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	//
	//

	//window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
	//					        SDL_WINDOWPOS_CENTERED, width, height,
	//					        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	//gLContext = SDL_GL_CreateContext(window);
	//
	//GLenum status = glewInit();
	//if (status != GLEW_OK)
	//{
	//	std::cerr << "Wystapil problem z inicjalizacja Opengl(glew)!" << std::endl;
	//	system("pause");
	//}

	//glEnable(GL_MULTISAMPLE);
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE); 
	////glEnable(GL_STENCIL_TEST);

	////glDepthFunc(GL_LESS);
	////glCullFace(GL_BACK);
	////glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	////SDL_SetRelativeMouseMode(SDL_TRUE);
	//isClosed = false;
}

void Display::Update()
{
	//SDL_GL_SwapWindow(window);
}

void Display::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

Display::~Display()
{
	/*SDL_GL_DeleteContext(gLContext);
	SDL_DestroyWindow(window);
	SDL_Quit();*/
}

