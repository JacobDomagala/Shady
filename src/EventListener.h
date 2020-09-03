#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include<Windows.h>
#include"DISPLAY\Display.h"
#include"DISPLAY\Camera.h"

struct EventListener{

	/*SDL_Event event;*/
	Display* windowHandle;
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

	EventListener(Display* windowHandle, Camera* camera, Shader* shaders);
	void Listen();	
};

#endif			