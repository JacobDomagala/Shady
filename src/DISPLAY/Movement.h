#ifndef MOVEMENT_H
#define MOVEMENT_H


#include<iomanip>
#include<glm/gtx/transform.hpp>

#include"Display.h"
#include"..\CLOCK\Clock.h"

class Display;
enum cameraMode{
	FLY,
	STATIC
};
class Movement
{
protected:
	Display* windowHandle;

	glm::mat4 viewMatrix;

	glm::vec2 oldMousePosition;
	glm::vec2 windowSize;
	
	glm::vec3 viewDirection;

	glm::vec3 position;
	glm::vec3 velocity;
	

	
	glm::vec3 lightPos;
	
	float deltaTime;
	Clock clock;
	

	glm::vec3 upVector;
	float MOUSE_SENSITIVITY;
	float MOVEMENT_SPEED;
	

	void Reset();
	float speedValue;
	bool flyMode;
public:

	
	
	
	Movement(Display* windowHandle, glm::vec3* light);
	

	glm::mat4 GetWorldToViewMatrix() const;
	glm::vec3 GetPosition() const;
	void mouseUpdate();
	
	void ComputeDelta();
	float GetDelta();
	float GetTime() { return clock.GetTime(); }


	

	void SetCameraMode(int mode);
	void Update();
	void KeyEvent();
	void IsOtherKeyPressed(int vKey);

	glm::vec3 GetLightPosition();
	void SetLightPosition(glm::vec3 lightPos);

	virtual void SetCamera(glm::vec3 cameraPosition, glm::vec3 viewDirection);
};

#endif