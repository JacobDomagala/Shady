#ifndef CAMERA_H
#define CAMERA_H

#include <iomanip>
#include <glm/gtx/transform.hpp>

#include "render/renderer.hpp"

using glm::vec2;
using glm::vec3;
using glm::mat4;

struct Display;

enum cameraMode
{
  FLY,
  STATIC
};

struct Camera
{
  friend struct EventListener;

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

  float MOVEMENT_SPEED;
  float speedValue;
  bool flyMode;
  void
  Reset();

  Camera(vec3 position);
  void
  SetCamera(vec3 cameraPosition, vec3 viewDirection);
  void
  ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true);

  void
  SetCameraMode(int mode);
  void
  Update(float deltaTime);
};

#endif