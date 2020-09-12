#include "Light.h"

Light::Light(glm::vec3 position, glm::vec3 color, lightType type)
{
   this->position = position;
   this->color = color;
   shadowTextureWidth = 4096;
   shadowTextureHeight = 4096;

   glm::mat4 projectionMatrix;
   modelMatrix = glm::mat4();

   switch (type)
   {
      case 0: {
         projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
      }
      case 1: {
         projectionMatrix = glm::perspective(60.0f, 1.0f, 0.1f, 100.0f);
      }
      case 2: {
         projectionMatrix = glm::perspective(60.0f, 1.0f, 0.1f, 100.0f);
      }
   }

   glm::mat4 viewMatrix = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

   biasMatrix = glm::translate(glm::vec3(0.5f)) * glm::scale(glm::vec3(0.5f)) * glm::mat4();

   lightSpaceMatrix = projectionMatrix * viewMatrix * modelMatrix;
   shadowMatrix = biasMatrix * lightSpaceMatrix;

   shadowTexture.CreateDepthBuffer(shadowTextureWidth, shadowTextureHeight);
}

void
Light::StartDrawingShadows(GLuint programID)
{
   glUseProgram(programID);
   glUniformMatrix4fv(glGetUniformLocation(programID, "lightSpaceMatrix"), 1, GL_FALSE,
                      glm::value_ptr(lightSpaceMatrix));

   glViewport(0, 0, shadowTextureWidth, shadowTextureHeight);

   glBindFramebuffer(GL_FRAMEBUFFER, shadowTexture.frameBufferID);
   glClear(GL_DEPTH_BUFFER_BIT);
}

void
Light::StopDrawingShadows()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
