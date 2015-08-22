#include "Clock.h"

int Clock::Initialize(){
	int result = QueryPerformanceFrequency(&clockFreq);
	if (!result)
		return false;

	return QueryPerformanceCounter(&ticksLastFrame);
	time = (float)ticksLastFrame.QuadPart / clockFreq.QuadPart;
}

void Clock::NewFrame(){
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	deltaTicks.QuadPart = tmp.QuadPart - ticksLastFrame.QuadPart;

	deltaTime = deltaTicks.QuadPart / (float)clockFreq.QuadPart;
	time += deltaTime;
	ticksLastFrame = tmp;
}
float Clock::GetDelta() const{
	
	return deltaTime;
	
}
void Clock::Wait(int ms){
	Sleep(ms);

}
float Clock::GetFps(){
	return (float)clockFreq.QuadPart / deltaTicks.QuadPart;
}