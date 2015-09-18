#include "Camera.h"


Camera::Camera(vec3* light) :
	
	viewDirection(0.0f, 0.0f, 1.0f),
	position(0.0f,0.0f,0.0f),
	MOVEMENT_SPEED(10.0f),
	velocity(0.0, 0.0, 0.0),
	lightPos(*light), 
	upVector(0.0, 1.0, 0.0),
	flyMode(false),
	speedValue(4.0)
{
	clock.Initialize();
	deltaTime = clock.GetDelta();
	viewMatrix = glm::lookAt(position, position + viewDirection, upVector);
}

glm::vec3 Camera::GetPosition() const
{
	return position;
}
glm::mat4 Camera::GetWorldToViewMatrix() const
{
	return viewMatrix;
}


void Camera::ComputeDelta()
{
	clock.NewFrame();
	deltaTime = clock.GetDelta();
}

void Camera::Reset()
{
	position = vec3(0.0f, 0.0f, 0.0f);
	viewDirection = vec3(1.0f, 0.0f, 0.0f);
	upVector = vec3(0.0, 1.0, 0.0);
}

void Camera::SetCameraMode(int mode)
{
	if (mode == FLY)
		flyMode = true;
	else if (mode == STATIC)
		flyMode = false;
}

void Camera::Update()
{
	ComputeDelta();
	if (flyMode)
	{	
		position += velocity * deltaTime * MOVEMENT_SPEED;
		viewMatrix = glm::lookAt(position, position + viewDirection, upVector);
	}
}

vec3 Camera::GetLightPosition()
{
	return lightPos;
}

void Camera::SetLightPosition(vec3 lightPos)
{
	this->lightPos = lightPos;
}

float Camera::GetDelta()
{
	return deltaTime;
}

void Camera::SetCamera(vec3 cameraPosition, vec3 viewDirection)
{
	position = cameraPosition;
	this->viewDirection = glm::normalize(viewDirection);
}
