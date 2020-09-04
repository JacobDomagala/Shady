#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include<Windows.h>
#include "app/window.hpp"
#include"DISPLAY\Camera.h"
#include "Shader.h"

struct EventListener{

	/*SDL_Event event;*/
	shady::app::Window* windowHandle;
	Camera* camera;
	Shader* shaders;

	vec2 windowSize;
	
	vec2 oldMousePosition;
	vec2 mousePosition;
	float MOUSE_SENSITIVITY;

	void KeyEvent();
	void MouseEvent();
	void SDLEvent();
	void IsOtherKeyPressed(int vKey);

	EventListener(shady::app::Window* windowHandle, Camera* camera, Shader* shaders);
	void Listen();	
};

#endif			