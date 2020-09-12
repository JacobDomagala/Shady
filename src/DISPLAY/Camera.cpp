#include "Camera.h"


Camera::Camera(vec3 position)
   : viewDirection(0.0f, 0.0f, 0.0f),
     position(position), //(0.0f, 0.0f, 0.0f),
     MOVEMENT_SPEED(10.0f),
     velocity(0.0f, 0.0f, 0.0f),
     worldUp(0.0f, 1.0f, 0.0f),
     flyMode(false),
     speedValue(4.0f),
     yaw(-90.0f),
     pitch(0.0f),
     mouseSensitivity(0.55f)
{
   viewMatrix = glm::lookAt(position, position + viewDirection, upVector);
}


void
Camera::Reset()
{
   position = vec3(0.0f, 0.0f, 0.0f);
   viewDirection = vec3(0.0f, 0.0f, 1.0f);
   upVector = vec3(0.0, 1.0, 0.0);
}

void
Camera::SetCameraMode(int mode)
{
   if (mode == FLY)
      flyMode = true;
   else if (mode == STATIC)
      flyMode = false;
}

void
Camera::Update(float deltaTime)
{
   if (flyMode)
   {
      /*	system("cls");
              std::cout << position.x << "  " << position.y << "  " << position.z << "\n";*/
      position += velocity * deltaTime * MOVEMENT_SPEED;
      viewMatrix = glm::lookAt(position, position + viewDirection, upVector);
   }
}

void
Camera::SetCamera(vec3 cameraPosition, vec3 viewDirection)
{
   position = cameraPosition;
   this->viewDirection = glm::normalize(viewDirection);
}
void
Camera::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch)
{
   xoffset *= mouseSensitivity;
   yoffset *= mouseSensitivity;

   yaw += xoffset;
   pitch += yoffset;

   if (constrainPitch)
   {
      if (pitch > 89.0f)
         pitch = 89.0f;
      if (pitch < -89.0f)
         pitch = -89.0f;
   }

   glm::vec3 front;
   viewDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
   viewDirection.y = sin(glm::radians(pitch));
   viewDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
   viewDirection = glm::normalize(viewDirection);

   rightVector = glm::normalize(glm::cross(viewDirection, worldUp));
   upVector = glm::normalize(glm::cross(rightVector, viewDirection));
}