#ifndef DISPLAY_H
#define DISPLAY_H

#include<SDL.h>
#include<windows.h>
#include<gtx\transform.hpp>
#include"..\SHADERS\Shader.h"

struct Display{
	friend struct EventListener;

	bool isClosed;
	int width, height;
	float fov, aspectRatio, fClip, nClip;
	glm::mat4 projectionMatrix;

	SDL_GLContext gLContext;
	SDL_Window* window;

	Display(int width, int height, const std::string& title);
	~Display();

	void Clear(float r, float g, float b, float a);
	void Update();
	
	void WrapMouse(bool choice){ SDL_SetWindowGrab(window, (SDL_bool)choice); }
	void ShowCursor(bool choice){ SDL_ShowCursor(choice); }
};

#endif