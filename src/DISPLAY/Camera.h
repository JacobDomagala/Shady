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
  float_t yaw;
  float_t pitch;
  // Camera options

  float_t mouseSensitivity;

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
  ProcessMouseMovement(float_t xoffset, float_t yoffset, bool constrainPitch = true);

  void
  SetCameraMode(int mode);
  void
  Update(float deltaTime);
};

#endif