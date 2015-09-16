#include "Display.h"

Display::Display(int width, int height, const std::string& title)
{	
	this->height = height;
	this->width = width;
	aspectRatio = (float)width / height;
	fov = 45.0;
	nClip = 0.1f;
	fClip = 100;
	SDL_Init(SDL_INIT_EVERYTHING);
	projectionMatrix = glm::perspective(fov,aspectRatio, nClip, fClip);
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
						        SDL_WINDOWPOS_CENTERED, width, height,
						        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	gLContext = SDL_GL_CreateContext(window);
	
	GLenum status = glewInit();
	if (status != GLEW_OK)
	{
		std::cerr << "Wystapil problem z inicjalizacja Opengl(glew)!" << std::endl;
		system("pause");
	}
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	
	isClosed = false;
}
bool Display::IsClosed()
{
	return isClosed;
}

void Display::Update()
{
	SDL_GL_SwapWindow(window);

	
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			isClosed = true;
		}
	
		else if (event.type == SDL_WINDOWEVENT)
		{
			switch (event.window.event)
			{
				case SDL_WINDOWEVENT_RESIZED:
				{
					width = event.window.data1;
					height = event.window.data2;
					projectionMatrix = glm::perspective(fov, aspectRatio, nClip, fClip);
					glViewport(0, 0, width, height);
					break;
				}
			}
		}
	}
	//Clear(0.2f, 0.2f, 0.2f, 0.0f);
}

void Display::Clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

Display::~Display()
{
	SDL_GL_DeleteContext(gLContext);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

glm::vec2 Display::GetWindowSize()
{
	int tmpHeight;
	int tmpWidth;
	
	SDL_GetWindowSize(window, &tmpWidth, &tmpHeight);
	
	return glm::vec2(tmpWidth, tmpHeight);
}