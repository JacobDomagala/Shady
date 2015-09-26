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

	vec3 upVector;
	vec3 rightVector;
	vec3 worldUp;
	// Eular Angles
	GLfloat yaw;
	GLfloat pitch;
	// Camera options
	
	GLfloat mouseSensitivity;

	mat4 viewMatrix;
	vec3 viewDirection;
	vec3 position;
	vec3 velocity;
	
	float deltaTime;

	
	float MOVEMENT_SPEED;
	float speedValue;
	bool flyMode;
	void Reset();
public:

	
	Camera(vec3 position);
	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true);

	mat4 GetWorldToViewMatrix() const;
	vec3 GetPosition() const;
	
	
	void ComputeDelta();
	float GetDelta();
	float GetTime() { return clock.GetTime(); }


	void SetCameraMode(int mode);
	void Update();


	virtual void SetCamera(vec3 cameraPosition, vec3 viewDirection);
};

#endif