#include "Display.h"




Display::Display(int width, int height, const std::string& title){
	
	this->height = height;
	this->width = width;
	aspectRatio = (float)width / height;
	fov = 45.0;
	nClip = 0.05;
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

	m_Window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
						SDL_WINDOWPOS_CENTERED, width, height,
						SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	m_GLContext = SDL_GL_CreateContext(m_Window);
	

	GLenum status = glewInit();
	if (status != GLEW_OK){
		std::cerr << "Wystapil problem z inicjalizacja Opengl(glew)!" << std::endl;
		system("pause");
	}
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	
	
	m_IsClosed = false;
	
	
}
bool Display::IsClosed()
{
	return m_IsClosed;
}

void Display::Update(){
	

	SDL_GL_SwapWindow(m_Window);

	if (GetAsyncKeyState(VK_ESCAPE)){
		m_IsClosed = true;
	}
	while (SDL_PollEvent(&m_Event)) {
		if (m_Event.type == SDL_QUIT){
			m_IsClosed = true;
		}
	
		else if (m_Event.type == SDL_WINDOWEVENT) {
			switch (m_Event.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
				width = m_Event.window.data1;
				height = m_Event.window.data2;
				projectionMatrix = glm::perspective(fov, aspectRatio, nClip, fClip);
				glViewport(0, 0, width, height);
				break;
			}
		}
	}
	Clear(1.04f, 1.04f, 1.08f, 0.0f);
}

void Display::Clear(float r, float g, float b, float a){
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

Display::~Display(){
	SDL_GL_DeleteContext(m_GLContext);
	SDL_DestroyWindow(m_Window);
	SDL_Quit();
}

glm::vec2 Display::GetWindowSize(){

	int tmpHeight;
	int tmpWidth;
	
	SDL_GetWindowSize(m_Window, &tmpWidth, &tmpHeight);
	
	return glm::vec2(tmpWidth, tmpHeight);
}