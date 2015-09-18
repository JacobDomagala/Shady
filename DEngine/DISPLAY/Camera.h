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
class Camera{
	friend class EventListener;
protected:
	Clock clock;

	mat4 viewMatrix;
	vec3 viewDirection;
	vec3 position;
	vec3 velocity;
	vec3 lightPos;
	
	float deltaTime;
	vec3 upVector;
	
	float MOVEMENT_SPEED;
	float speedValue;
	bool flyMode;
	void Reset();
public:

	
	Camera(vec3* light);
	

	mat4 GetWorldToViewMatrix() const;
	vec3 GetPosition() const;
	
	
	void ComputeDelta();
	float GetDelta();
	float GetTime() { return clock.GetTime(); }


	void SetCameraMode(int mode);
	void Update();


	vec3 GetLightPosition();
	void SetLightPosition(vec3 lightPos);

	virtual void SetCamera(vec3 cameraPosition, vec3 viewDirection);
};

#endif