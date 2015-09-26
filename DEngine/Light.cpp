#include "Light.h"

Light::Light(glm::vec3 position, glm::vec3 color, lightType type)
{
	this->position = position;
	this->color = color;
	glm::mat4 projectionMatrix;

	switch (type)
	{
		case 0:
		{
			 projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 20.0f);
		}
		case 1:
		{
			 projectionMatrix = glm::perspective(60.0f, 1.0f, 0.1f, 100.0f);
		}
		case 2:
		{
			 projectionMatrix = glm::perspective(60.0f, 1.0f, 0.1f, 100.0f);
		}
	}

	glm::mat4 viewMatrix = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	biasMatrix = 
	{0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	};
	lightSpaceMatrix = projectionMatrix * viewMatrix;
	shadowMatrix = biasMatrix * projectionMatrix * viewMatrix;

	shadowTexture.CreateDepthBuffer(2048, 2048);

}

void Light::StartDrawingShadows(GLuint programID)
{
	glUseProgram(programID);
	glUniformMatrix4fv(glGetUniformLocation(programID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	glViewport(0, 0, 2048, 2048);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowTexture.frameBufferID);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Light::StopDrawingShadows()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

