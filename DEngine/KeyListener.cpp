#include "KeyListener.h"

KeyListener::KeyListener(Display* windowHandle, Camera* camera):
	windowHandle(windowHandle),
	camera(camera)
{

}

void KeyListener::KeyEvent()
{
	if (GetAsyncKeyState(VK_ESCAPE))
	{
		windowHandle->isClosed = true;
	}
	if (GetAsyncKeyState(VK_SHIFT))
	{
		camera->velocity *= camera->speedValue;
	}
	if (GetAsyncKeyState(VK_SPACE))
	{
		camera->velocity.y = camera->speedValue;
	}
	if (!GetAsyncKeyState(0x57) && !GetAsyncKeyState(0x53) && !GetAsyncKeyState(0x41) &&
		!GetAsyncKeyState(0x44) && !GetAsyncKeyState(VK_SPACE))
	{
		camera->velocity *= 0.0f;
	}
	//Move forward (W)
	if (GetAsyncKeyState(0x57))
	{
		camera->velocity = camera->viewDirection;
		IsOtherKeyPressed(0x57);
		if (GetAsyncKeyState(VK_SHIFT))
		{
			camera->velocity *= camera->speedValue;
		}
	}
	//Move backward (S)
	if (GetAsyncKeyState(0x53))
	{
		camera->velocity = -camera->viewDirection;
		IsOtherKeyPressed(0x53);
		if (GetAsyncKeyState(VK_SHIFT))
		{
			camera->velocity *= camera->speedValue;
		}
	}
	//Move left (A)
	if (GetAsyncKeyState(0x41))
	{
		vec3 tmp = glm::cross(camera->viewDirection, camera->upVector);
		camera->velocity = tmp * -2.0f;
		IsOtherKeyPressed(0x41);
	}
	//Move right (D)
	if (GetAsyncKeyState(0x44))
	{
		vec3 tmp = glm::cross(camera->viewDirection, camera->upVector);
		camera->velocity = tmp * 2.0f;
		IsOtherKeyPressed(0x44);
	}
	//Reset camera to default (R)
	if (GetAsyncKeyState(0x52))
	{
		camera->position = vec3(5.0f, -1.5f, 0.0f);
	}
	//Move light forward (T)
	if (GetAsyncKeyState(0x54))
	{
		camera->lightPos += vec3(0.2, 0.0, 0.2);
	}
	//Move light backward (G)
	if (GetAsyncKeyState(0x47))
	{
		camera->lightPos -= vec3(0.2, 0.0, 0.2);
	}
}
void KeyListener::IsOtherKeyPressed(int vKey)
{
	//W
	if (vKey != 0x57 && GetAsyncKeyState(0x57))
	{
		camera->velocity += camera->viewDirection;
	}
	//S
	if (vKey != 0x53 && GetAsyncKeyState(0x53))
	{
		camera->velocity += -camera->viewDirection;
	}
	//A
	if (vKey != 0x41 && GetAsyncKeyState(0x41))
	{
		glm::vec3 tmp = glm::cross(camera->viewDirection, camera->upVector);
		camera->velocity += tmp;
	}
	//D
	if (vKey != 0x44 && GetAsyncKeyState(0x44))
	{
		glm::vec3 tmp = glm::cross(camera->viewDirection, camera->upVector);
		camera->velocity += -tmp;
	}
	//ESC
	if (vKey != VK_SPACE && GetAsyncKeyState(VK_SPACE))
	{
		camera->velocity.y = 1.0f;
	}
}