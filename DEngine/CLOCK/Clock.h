#ifndef CLOCK_H
#define CLOCK_H

#include<windows.h>
#include<iostream>


class Clock
{
private:
	LARGE_INTEGER clockFreq;
	LARGE_INTEGER ticksLastFrame; //time elapsed durning last frame
	LARGE_INTEGER deltaTicks;
	float deltaTime;
	float time;

public:

	

	
	int Initialize();
	
	void NewFrame();
	float GetDelta() const;
	void Wait(int ms);
	float GetFps();
	float GetTime() { return time; }
};

#endif