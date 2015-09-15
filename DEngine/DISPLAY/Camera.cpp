#include "Camera.h"


Camera::Camera(Display* windowHandle, glm::vec3* light) :
	windowSize(windowHandle->GetWindowSize().x, windowHandle->GetWindowSize().y),
	viewDirection(0.0f, 0.0f, 1.0f),
	position(0.0f,0.0f,0.0f),
	oldMousePosition(windowHandle->GetWindowSize().x / 2, windowHandle->GetWindowSize().y / 2),
	MOUSE_SENSITIVITY(0.001f),
	MOVEMENT_SPEED(10.0f),
	velocity(0.0, 0.0, 0.0),
	lightPos(*light), 
	upVector(0.0, 1.0, 0.0),
	windowHandle(windowHandle),
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

void Camera::mouseUpdate()
{
	int x, y;
	SDL_GetMouseState(&x, &y);
	vec2 mousePosition(x,y);
	windowSize = vec2(windowHandle->GetWindowSize().x, windowHandle->GetWindowSize().y);
	
	if (x > windowSize.x - 20 || x < 10)
		SDL_WarpMouseInWindow(windowHandle->window, windowSize.x / 2, windowSize.y / 2);
	if (y > windowSize.y - 20 || y < 10)
		SDL_WarpMouseInWindow(windowHandle->window, windowSize.x / 2, windowSize.y / 2);
	

	vec2 mouseDelta = oldMousePosition - mousePosition;
	float lenght = glm::length(mouseDelta);
	if (lenght > windowSize.y/3)
	{
		oldMousePosition = mousePosition;
		return;
	}
	
	vec3 rotateAround = glm::cross(viewDirection, upVector);
	mat4 rotation = glm::rotate(mouseDelta.x * MOUSE_SENSITIVITY, upVector) *
		            glm::rotate(mouseDelta.y * MOUSE_SENSITIVITY, rotateAround);


	viewDirection = glm::normalize(glm::mat3(rotation) * viewDirection);
	
	
	oldMousePosition = mousePosition;
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
		mouseUpdate();
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
