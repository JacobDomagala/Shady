#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include<Windows.h>
#include"DISPLAY\Display.h"
#include"DISPLAY\Camera.h"

class EventListener{

private:

	SDL_Event event;
	Display* windowHandle;
	Camera* camera;
	float MOUSE_SENSITIVITY;
	vec2 oldMousePosition;
	vec2 windowSize;
	
	vec2 mousePosition;


	void KeyEvent();
	void MouseEvent();
	void SDLEvent();
	void IsOtherKeyPressed(int vKey);
public:
	EventListener(Display* windowHandle, Camera* camera);
	void Listen();
	
};

#endif			