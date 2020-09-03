#ifndef CLOCK_H
#define CLOCK_H

#include<windows.h>
#include<iostream>


struct Clock
{
	Clock();
	LARGE_INTEGER clockFreq;
	LARGE_INTEGER ticksLastFrame; //time elapsed durning last frame
	LARGE_INTEGER deltaTicks;
	float deltaTime;
	float time;
	
	void NewFrame();
	void Wait(int ms);
};

#endif