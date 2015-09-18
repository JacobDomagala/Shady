#include "EventListener.h"

EventListener::EventListener(Display* windowHandle, Camera* camera):
	camera(camera),
	windowSize(windowHandle->GetWindowSize().x, windowHandle->GetWindowSize().y),
	windowHandle(windowHandle),
	MOUSE_SENSITIVITY(0.001f),
	oldMousePosition(windowHandle->GetWindowSize().x / 2, windowHandle->GetWindowSize().y / 2)
{

}

void EventListener::KeyEvent()
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
		camera->position = vec3(0.0f, 0.0f, 0.0f);
	}
	//Move light forward (T)
	if (GetAsyncKeyState(0x54))
	{
		camera->lightPos += vec3(0.2f, 0.0f, 0.2f);
	}
	//Move light backward (G)
	if (GetAsyncKeyState(0x47))
	{
		camera->lightPos -= vec3(0.2f, 0.0f, 0.2f);
	}
}

void EventListener::SDLEvent()
{
	while (SDL_PollEvent(&this->event))
	{
		
		if (this->event.type == SDL_QUIT)
		{
			windowHandle->isClosed = true;
		}
		else if (this->event.type == SDL_WINDOWEVENT)
		{
			switch (this->event.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
			{
				windowHandle->width = this->event.window.data1;
				windowHandle->height = this->event.window.data2;
				windowHandle->projectionMatrix = glm::perspective(windowHandle->fov, windowHandle->aspectRatio, windowHandle->nClip, windowHandle->fClip);
				glViewport(0, 0, windowHandle->width, windowHandle->height);
				break;
			}
			}
		}
	}
}
void EventListener::IsOtherKeyPressed(int vKey)
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

void EventListener::MouseEvent()
{
	
	int x, y;
	SDL_GetMouseState(&x, &y);
	vec2 mousePosition(x, y);
	windowSize = vec2(windowHandle->GetWindowSize().x, windowHandle->GetWindowSize().y);


	if (x > windowSize.x - 20 || x < 10)
		SDL_WarpMouseInWindow(windowHandle->window, windowSize.x / 2, windowSize.y / 2);
	if (y > windowSize.y - 20 || y < 10)
		SDL_WarpMouseInWindow(windowHandle->window, windowSize.x / 2, windowSize.y / 2);

	vec2 mouseDelta = oldMousePosition - mousePosition;
	float lenght = glm::length(mouseDelta);
	if (lenght > windowSize.y / 3)
	{
		oldMousePosition = mousePosition;
		return;
	}

	vec3 rotateAround = glm::cross(camera->viewDirection, camera->upVector);
	mat4 rotation = glm::rotate(mouseDelta.x * MOUSE_SENSITIVITY, camera->upVector) *
		glm::rotate(mouseDelta.y * MOUSE_SENSITIVITY, rotateAround);


	camera->viewDirection = glm::normalize(glm::mat3(rotation) * camera->viewDirection);


	oldMousePosition = mousePosition;
}

void EventListener::Listen()
{
	KeyEvent();
	MouseEvent();
	SDLEvent();
}