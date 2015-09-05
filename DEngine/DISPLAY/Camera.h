#ifndef CAMERA_H
#define CAMERA_H


#include<iomanip>
#include<gtx\transform.hpp>

#include"Display.h"
#include"..\CLOCK\Clock.h"

using glm::vec2;
using glm::vec3;
using glm::mat4;

class Display;
enum cameraMode{
	FLY,
	STATIC
};
class Camera
{
protected:
	Display* windowHandle;

	mat4 viewMatrix;

	vec2 oldMousePosition;
	vec2 windowSize;
	
	vec3 viewDirection;

	vec3 position;
	vec3 velocity;
	

	
	vec3 lightPos;
	
	float deltaTime;
	Clock clock;
	

	vec3 upVector;
	float MOUSE_SENSITIVITY;
	float MOVEMENT_SPEED;
	

	void Reset();
	float speedValue;
	bool flyMode;
public:

	
	
	
	Camera(Display* windowHandle, vec3* light);
	

	mat4 GetWorldToViewMatrix() const;
	vec3 GetPosition() const;
	void mouseUpdate();
	
	void ComputeDelta();
	float GetDelta();
	float GetTime() { return clock.GetTime(); }


	

	void SetCameraMode(int mode);
	void Update();
	void KeyEvent();
	void IsOtherKeyPressed(int vKey);

	vec3 GetLightPosition();
	void SetLightPosition(vec3 lightPos);

	virtual void SetCamera(vec3 cameraPosition, vec3 viewDirection);
};

#endif