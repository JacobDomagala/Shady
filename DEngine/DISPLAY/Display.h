#ifndef DISPLAY_H
#define DISPLAY_H

#include<SDL.h>
#include<windows.h>
#include<gtx\transform.hpp>
#include"..\SHADERS\Shader.h"

class Display
{
public:
	
	Display(int width, int height, const std::string& title);
	~Display(); 
	
	bool IsClosed();
	void Clear(float r, float g, float b, float a);
	
	void Update();
	bool m_IsClosed;
	
	glm::vec2 GetWindowSize();
	SDL_Window* m_Window;

	void WrapMouse(bool choice){ SDL_SetWindowGrab(m_Window, (SDL_bool)choice); }
	void ShowCursor(bool choice){ SDL_ShowCursor(choice); }
	glm::mat4 GetProjection() { return projectionMatrix; }
protected:
private:
	int width, height;
	float fov, aspectRatio, fClip, nClip;
	glm::mat4 projectionMatrix;
	
	SDL_Event m_Event;
	
	SDL_GLContext m_GLContext;

	
};

#endif