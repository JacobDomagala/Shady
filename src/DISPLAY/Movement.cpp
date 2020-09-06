#include "Movement.h"


Movement::Movement(Display *windowHandle, glm::vec3 *light)
  :// windowSize(windowHandle->GetWindowSize().x, windowHandle->GetWindowSize().y),
    viewDirection(0.0f, 0.0f, 1.0f), position(5.0f, -1.5f, 0.0f),
    // oldMousePosition(windowHandle->GetWindowSize().x / 2, windowHandle->GetWindowSize().y / 2),
    MOUSE_SENSITIVITY(0.001f), MOVEMENT_SPEED(2.0f), velocity(0.0, 0.0, 0.0), lightPos(*light),
    upVector(0.0, 1.0, 0.0), windowHandle(windowHandle), flyMode(false), speedValue(4.0)
{
  viewMatrix = glm::lookAt(position, position + viewDirection, upVector);
}

glm::vec3
Movement::GetPosition() const
{
  return position;
}
glm::mat4
Movement::GetWorldToViewMatrix() const
{
  return viewMatrix;
}

void
Movement::mouseUpdate()
{
  // int x, y;
  // SDL_GetMouseState(&x, &y);
  // glm::vec2 mousePosition(x,y);
  // windowSize = glm::vec2(windowHandle->GetWindowSize().x, windowHandle->GetWindowSize().y);
  //
  // if (x > windowSize.x - 20 || x < 10)
  //	SDL_WarpMouseInWindow(windowHandle->m_Window, windowSize.x / 2, windowSize.y / 2);
  // if (y > windowSize.y - 20 || y < 10)
  //	SDL_WarpMouseInWindow(windowHandle->m_Window, windowSize.x / 2, windowSize.y / 2);
  //

  // glm::vec2 mouseDelta = oldMousePosition - mousePosition;
  // float lenght = glm::length(mouseDelta);
  // if (lenght > windowSize.y/3){
  //	oldMousePosition = mousePosition;
  //	return;
  //}
  //
  // glm::vec3 rotateAround = glm::cross(viewDirection, upVector);
  // glm::mat4 rotation = glm::rotate(mouseDelta.x * MOUSE_SENSITIVITY, upVector) *
  //	                 glm::rotate(mouseDelta.y * MOUSE_SENSITIVITY, rotateAround);


  // viewDirection = glm::normalize(glm::mat3(rotation) * viewDirection);
  //
  //
  // oldMousePosition = mousePosition;
}

void
Movement::ComputeDelta()
{
  deltaTime = clock.ToggleTimer().GetMilliseconds().count();
}


void
Movement::Reset()
{
  position = glm::vec3(0.0f, -1.5f, 0.0f);
  viewDirection = glm::vec3(1.0f, 0.0f, 0.0f);
  upVector = glm::vec3(0.0, 1.0, 0.0);
}
void
Movement::SetCameraMode(int mode)
{
  if (mode == FLY)
    flyMode = true;
  else if (mode == STATIC)
    flyMode = false;
}
void
Movement::Update()
{
  /*system("cls");
        std::cout << position.x<<" "<<position.y<<" "<<position.z;*/
  ComputeDelta();
  if (flyMode) {

    mouseUpdate();
    KeyEvent();
    position += velocity * deltaTime * MOVEMENT_SPEED;


    viewMatrix = glm::lookAt(position, position + viewDirection, upVector);
  }
}

void
Movement::KeyEvent()
{

  if (GetAsyncKeyState(VK_SHIFT)) velocity *= speedValue;

  if (GetAsyncKeyState(VK_SPACE)) velocity.y = speedValue;

  if (!GetAsyncKeyState(0x57) && !GetAsyncKeyState(0x53) && !GetAsyncKeyState(0x41)
      && !GetAsyncKeyState(0x44) && !GetAsyncKeyState(VK_SPACE))
    velocity *= 0.0f;


  if (GetAsyncKeyState(0x57)) {


    velocity = viewDirection;

    IsOtherKeyPressed(0x57);
    if (GetAsyncKeyState(VK_SHIFT)) velocity *= speedValue;
  }

  if (GetAsyncKeyState(0x53)) {


    velocity = -viewDirection;

    IsOtherKeyPressed(0x53);
    if (GetAsyncKeyState(VK_SHIFT)) velocity *= speedValue;
  }


  if (GetAsyncKeyState(0x41)) {

    glm::vec3 tmp = glm::cross(viewDirection, upVector);

    velocity = tmp * -2.0f;

    IsOtherKeyPressed(0x41);
  }

  if (GetAsyncKeyState(0x44)) {// D

    glm::vec3 tmp = glm::cross(viewDirection, upVector);

    velocity = tmp * 2.0f;

    IsOtherKeyPressed(0x44);
  }

  if (GetAsyncKeyState(0x52))// R
    Reset();
}

void
Movement::IsOtherKeyPressed(int vKey)
{

  // W
  if (vKey != 0x57 && GetAsyncKeyState(0x57)) velocity += viewDirection;


  // S
  if (vKey != 0x53 && GetAsyncKeyState(0x53)) velocity += -viewDirection;


  // A
  if (vKey != 0x41 && GetAsyncKeyState(0x41)) {
    glm::vec3 tmp = glm::cross(viewDirection, upVector);
    velocity += tmp;
  }

  // D
  if (vKey != 0x44 && GetAsyncKeyState(0x44)) {
    glm::vec3 tmp = glm::cross(viewDirection, upVector);
    velocity += -tmp;
  }
  if (vKey != VK_SPACE && GetAsyncKeyState(VK_SPACE)) velocity.y = 1.0f;
}
glm::vec3
Movement::GetLightPosition()
{
  return lightPos;
}
void
Movement::SetLightPosition(glm::vec3 lightPos)
{
  this->lightPos = lightPos;
}
float
Movement::GetDelta()
{
  return deltaTime;
}
void
Movement::SetCamera(glm::vec3 cameraPosition, glm::vec3 viewDirection)
{
  position = cameraPosition;
  this->viewDirection = glm::normalize(viewDirection);
}
