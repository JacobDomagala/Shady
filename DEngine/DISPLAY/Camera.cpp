#include "Camera.h"


Camera::Camera(Display* windowHandle, glm::vec3* light) :
	windowSize(windowHandle->GetWindowSize().x, windowHandle->GetWindowSize().y),
	viewDirection(0.0f, 0.0f, 1.0f),
	position(5.0f,-1.5f,0.0f),
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
		SDL_WarpMouseInWindow(windowHandle->m_Window, windowSize.x / 2, windowSize.y / 2);
	if (y > windowSize.y - 20 || y < 10)
		SDL_WarpMouseInWindow(windowHandle->m_Window, windowSize.x / 2, windowSize.y / 2);
	

	vec2 mouseDelta = oldMousePosition - mousePosition;
	float lenght = glm::length(mouseDelta);
	if (lenght > windowSize.y/3){
		oldMousePosition = mousePosition;
		return;
	}
	
	vec3 rotateAround = glm::cross(viewDirection, upVector);
	mat4 rotation = glm::rotate(mouseDelta.x * MOUSE_SENSITIVITY, upVector) *
		                 glm::rotate(mouseDelta.y * MOUSE_SENSITIVITY, rotateAround);


	viewDirection = glm::normalize(glm::mat3(rotation) * viewDirection);
	
	
	oldMousePosition = mousePosition;
}

void Camera::ComputeDelta(){
	
	clock.NewFrame();
	deltaTime = clock.GetDelta();
	
}


void Camera::Reset(){
	position = vec3(0.0f, -1.5f, 0.0f);
	viewDirection = vec3(1.0f, 0.0f, 0.0f);
	upVector = vec3(0.0, 1.0, 0.0);
}
void Camera::SetCameraMode(int mode){
	if (mode == FLY)
		flyMode = true;
	else if (mode == STATIC)
		flyMode = false;
}
void Camera::Update(){
	//system("cls");
	//std::cout << position.x<<" "<<position.y<<" "<<position.z<<"\n\n";
	//std::cout << lightPos.x << " " << lightPos.y << " " << lightPos.z;
	ComputeDelta();
	if (flyMode){
		
		mouseUpdate();
		KeyEvent();
		position += velocity * deltaTime * MOVEMENT_SPEED;
		

		viewMatrix = glm::lookAt(position, position + viewDirection, upVector);
	}

}

void Camera::KeyEvent(){
	
	if (GetAsyncKeyState(VK_SHIFT))
		velocity *= speedValue;

	if (GetAsyncKeyState(VK_SPACE))
		velocity.y = speedValue;
	
	if (!GetAsyncKeyState(0x57) && !GetAsyncKeyState(0x53) && !GetAsyncKeyState(0x41) &&
		!GetAsyncKeyState(0x44) && !GetAsyncKeyState(VK_SPACE))
			velocity *= 0.0f;
	
	//Move forward (W)
	if (GetAsyncKeyState(0x57))	{
		velocity = viewDirection;
			IsOtherKeyPressed(0x57);
			if (GetAsyncKeyState(VK_SHIFT)) 
				velocity *= speedValue;
	}
	//Move backward (S)
	if (GetAsyncKeyState(0x53)){
		velocity = -viewDirection;
			IsOtherKeyPressed(0x53);
			if (GetAsyncKeyState(VK_SHIFT))  
				velocity *= speedValue;
	}

	//Move left (A)
	if (GetAsyncKeyState(0x41)) {

		vec3 tmp = glm::cross(viewDirection, upVector);
		velocity = tmp * -2.0f;
		IsOtherKeyPressed(0x41);
	}
	
	//Move right (D)
	if (GetAsyncKeyState(0x44)){      
		
		vec3 tmp = glm::cross(viewDirection, upVector);
		velocity = tmp * 2.0f;
		IsOtherKeyPressed(0x44);
	}

	//Reset camera to default (R)
	if (GetAsyncKeyState(0x52)) 
		position = vec3(5.0f, -1.5f, 0.0f);
	
	//Move light forward (T)
	if (GetAsyncKeyState(0x54)) {

		lightPos += vec3(0.2, 0.0, 0.2);
	}
	//Move light backward (G)
	if (GetAsyncKeyState(0x47)) {

		lightPos -= vec3(0.2, 0.0, 0.2);
	}
}

void Camera::IsOtherKeyPressed(int vKey){

	//W
	if (vKey != 0x57 && GetAsyncKeyState(0x57))
		velocity += viewDirection;
	

	//S
	if (vKey != 0x53 && GetAsyncKeyState(0x53))
		velocity += -viewDirection;
	
	

	//A
	if (vKey != 0x41 && GetAsyncKeyState(0x41)){
		glm::vec3 tmp = glm::cross(viewDirection, upVector);
		velocity += tmp;
		
	}

	//D
	if (vKey != 0x44 && GetAsyncKeyState(0x44)){
		glm::vec3 tmp = glm::cross(viewDirection, upVector);
		velocity += -tmp;
		
	}
	if (vKey != VK_SPACE && GetAsyncKeyState(VK_SPACE))
		velocity.y = 1.0f;
		
	
	
}
vec3 Camera::GetLightPosition(){
	return lightPos;
}
void Camera::SetLightPosition(vec3 lightPos){
	this->lightPos = lightPos;
}
float Camera::GetDelta(){
	return deltaTime;
}
void Camera::SetCamera(vec3 cameraPosition, vec3 viewDirection){
	position = cameraPosition;
	this->viewDirection = glm::normalize(viewDirection);

}
