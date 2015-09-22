#include "Camera.h"


Camera::Camera(vec3* light) :
	viewDirection(0.0f, 0.0f, 1.0f),
	position(0.0f, 0.0f, 0.0f),
	MOVEMENT_SPEED(10.0f),
	velocity(0.0f, 0.0f, 0.0f),
	lightPos(*light), 
	worldUp(0.0f, 1.0f, 0.0f),
	flyMode(false),
	speedValue(4.0f),
	yaw(-90.0f),
	pitch(0.0f),
	mouseSensitivity(0.55f)
{
	clock.Initialize();
	deltaTime = clock.GetDelta();
	viewMatrix = glm::lookAt(position, position + viewDirection, upVector);
}

vec3 Camera::GetPosition() const
{
	return position;
}

mat4 Camera::GetWorldToViewMatrix() const
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
	viewDirection = vec3(0.0f, 0.0f, 1.0f);
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
		system("cls");
		std::cout << viewDirection.x << "   " << viewDirection.y << "\n";

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
void Camera::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch)
{
	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}

	// Update Front, Right and Up Vectors using the updated Eular angles
	glm::vec3 front;
	viewDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	viewDirection.y = sin(glm::radians(pitch));
	viewDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	viewDirection = glm::normalize(viewDirection);
	// Also re-calculate the Right and Up vector
	rightVector = glm::normalize(glm::cross(viewDirection, worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	upVector = glm::normalize(glm::cross(rightVector, viewDirection));
}