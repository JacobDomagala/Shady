#include "Clock.h"

Clock::Clock()
{
	QueryPerformanceFrequency(&clockFreq);
	time = 0.0f;
	QueryPerformanceCounter(&ticksLastFrame);
}

void Clock::NewFrame()
{
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	deltaTicks.QuadPart = tmp.QuadPart - ticksLastFrame.QuadPart;

	deltaTime = deltaTicks.QuadPart / (float)clockFreq.QuadPart;
	time += deltaTime;
	ticksLastFrame = tmp;
}

void Clock::Wait(int ms)
{
	Sleep(ms);
}

