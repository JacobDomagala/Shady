#ifndef KEYLISTENER_H
#define KEYLISTENER_H

#include<Windows.h>
#include"DISPLAY\Display.h"
#include"DISPLAY\Camera.h"

class KeyListener{

private:
	
	Display* windowHandle;
	Camera* camera;
public:
	KeyListener(Display* windowHandle, Camera* camera);
	void KeyEvent();
	void IsOtherKeyPressed(int vKey);
};

#endif			