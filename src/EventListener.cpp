#include "EventListener.h"

EventListener::EventListener(shady::app::Window *windowHandle, Camera *camera, Shader *shaders)
  : camera(camera),
    // windowSize(windowHandle->width, windowHandle->height),
    windowHandle(windowHandle),
    // oldMousePosition(windowHandle->width / 2, windowHandle->height / 2),
    shaders(shaders)
{}

void
EventListener::KeyEvent()
{
  // if (GetAsyncKeyState(VK_ESCAPE)) {
  //   // windowHandle->isClosed = true;
  // }
  // if (GetAsyncKeyState(VK_SHIFT)) { camera->velocity *= camera->speedValue; }
  // if (GetAsyncKeyState(VK_SPACE)) { camera->velocity.y = camera->speedValue; }
  // if (!GetAsyncKeyState('W') && !GetAsyncKeyState('S') && !GetAsyncKeyState('A')
  //     && !GetAsyncKeyState('D') && !GetAsyncKeyState(VK_SPACE)) {
  //   camera->velocity *= 0.0f;
  // }
  // // Move forward (W)
  // if (GetAsyncKeyState('W')) {
  //   camera->velocity = camera->viewDirection;
  //   IsOtherKeyPressed('W');
  //   if (GetAsyncKeyState(VK_SHIFT)) { camera->velocity *= camera->speedValue; }
  // }
  // // Move backward (S)
  // if (GetAsyncKeyState('S')) {
  //   camera->velocity = -camera->viewDirection;
  //   IsOtherKeyPressed('S');
  //   if (GetAsyncKeyState(VK_SHIFT)) { camera->velocity *= camera->speedValue; }
  // }
  // // Move left (A)
  // if (GetAsyncKeyState('A')) {
  //   vec3 tmp = glm::cross(camera->viewDirection, camera->upVector);
  //   camera->velocity = tmp * -2.0f;
  //   IsOtherKeyPressed('A');
  // }
  // // Move right (D)
  // if (GetAsyncKeyState('D')) {
  //   vec3 tmp = glm::cross(camera->viewDirection, camera->upVector);
  //   camera->velocity = tmp * 2.0f;
  //   IsOtherKeyPressed('D');
  // }
  // // Reset camera to default (R)
  // if (GetAsyncKeyState('R')) { camera->position = vec3(0.0f, 0.0f, 0.0f); }
  // // Move light forward (T)
  // if (GetAsyncKeyState('T')) {}
  // // Move light backward (G)
  // if (GetAsyncKeyState('G')) {}
  // // Reset position
  // if (GetAsyncKeyState(0x52)) { camera->Reset(); }

  // // DEBUG
  // if (GetAsyncKeyState('1')) {
  //   // shaders->LoadShaders("./SHADERS/SOURCE/SimpleShader_vs.glsl",
  //   //	"./SHADERS/SOURCE/SimpleShader_fs.glsl");
  // }
  // if (GetAsyncKeyState('2')) {
  //   shaders->LoadShaders("./SHADERS/SOURCE/NoNormal_vs.glsl", "./SHADERS/SOURCE/NoNormal_fs.glsl");
  // }
  // if (GetAsyncKeyState(0x33)) { glDisable(GL_MULTISAMPLE); }
  // if (GetAsyncKeyState(0x34)) { glEnable(GL_MULTISAMPLE); }
}

void
EventListener::SDLEvent()
{
  // while (SDL_PollEvent(&event))
  //{
  //
  //	if (event.type == SDL_QUIT)
  //	{
  //		windowHandle->isClosed = true;
  //	}
  //	if (event.type == SDL_WINDOWEVENT)
  //	{
  //		switch (event.window.event)
  //		{
  //			case SDL_WINDOWEVENT_RESIZED:
  //			{
  //				windowHandle->width = event.window.data1;
  //				windowHandle->height = event.window.data2;
  //				windowHandle->projectionMatrix = glm::perspective(windowHandle->fov,
  //windowHandle->aspectRatio, windowHandle->nClip, windowHandle->fClip); 				glViewport(0, 0,
  //windowHandle->width, windowHandle->height); 				break;
  //			}
  //		}
  //	}
  //}
}
void
EventListener::IsOtherKeyPressed(int vKey)
{
  // // W
  // if (vKey != 0x57 && GetAsyncKeyState(0x57)) { camera->velocity += camera->viewDirection; }
  // // S
  // if (vKey != 0x53 && GetAsyncKeyState(0x53)) { camera->velocity += -camera->viewDirection; }
  // // A
  // if (vKey != 0x41 && GetAsyncKeyState(0x41)) {
  //   glm::vec3 tmp = glm::cross(camera->viewDirection, camera->upVector);
  //   camera->velocity += tmp;
  // }
  // // D
  // if (vKey != 0x44 && GetAsyncKeyState(0x44)) {
  //   glm::vec3 tmp = glm::cross(camera->viewDirection, camera->upVector);
  //   camera->velocity += -tmp;
  // }

  // if (vKey != VK_SPACE && GetAsyncKeyState(VK_SPACE)) { camera->velocity.y = 1.0f; }
}

void
EventListener::MouseEvent()
{
  // int x, y;
  // SDL_GetMouseState(&x, &y);
  // mousePosition = vec2(x, y);

  // vec2 mouseDelta;
  // mouseDelta.x = mousePosition.x - oldMousePosition.x;
  // mouseDelta.y = oldMousePosition.y - mousePosition.y;

  // float mouseDeltaLenght = glm::length(mouseDelta);
  // if (mouseDeltaLenght > 100.0f)
  //{
  //	oldMousePosition = mousePosition;
  //	return;
  //}
  // camera->ProcessMouseMovement(mouseDelta.x, mouseDelta.y);
  // oldMousePosition = mousePosition;
}

void
EventListener::Listen()
{
  MouseEvent();
  KeyEvent();
  SDLEvent();
}