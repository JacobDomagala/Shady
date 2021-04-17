#include "defered_pipeline.hpp"
#include "scene/perspective_camera.hpp"
#include "vulkan_buffer.hpp"
#include "vulkan_common.hpp"


#include <fmt/format.h>
#include <glm/glm.hpp>

namespace shady::render::vulkan {

float timer = 0.0f;

struct Light
{
   glm::vec4 position;
   glm::vec3 color;
   float radius;
};

struct
{
   glm::mat4 projection;
   glm::mat4 model;
   glm::mat4 view;
   glm::vec4 instancePos[3];
} uboOffscreenVS;

struct
{
   Light lights[6];
   glm::vec4 viewPos;
   int debugDisplayTarget = 0;
} uboComposition;

// Update matrices used for the offscreen rendering of the scene
void
DeferedPipeline::UpdateUniformBufferOffscreen()
{
   uboOffscreenVS.projection = m_camera->GetProjection();
   uboOffscreenVS.view = m_camera->GetView();
   uboOffscreenVS.model = glm::mat4(1.0f);
   m_offscreenBuffer.CopyData(&uboOffscreenVS);
   // memcpy(m_offscreenBuffer.mapped, , sizeof(uboOffscreenVS));
}

// Update lights and parameters passed to the composition shaders
void
DeferedPipeline::UpdateUniformBufferComposition()
{
   // White
   uboComposition.lights[0].position = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
   uboComposition.lights[0].color = glm::vec3(1.5f);
   uboComposition.lights[0].radius = 15.0f * 0.25f;
   // Red
   uboComposition.lights[1].position = glm::vec4(-2.0f, 0.0f, 0.0f, 0.0f);
   uboComposition.lights[1].color = glm::vec3(1.0f, 0.0f, 0.0f);
   uboComposition.lights[1].radius = 15.0f;
   // Blue
   uboComposition.lights[2].position = glm::vec4(2.0f, -1.0f, 0.0f, 0.0f);
   uboComposition.lights[2].color = glm::vec3(0.0f, 0.0f, 2.5f);
   uboComposition.lights[2].radius = 5.0f;
   // Yellow
   uboComposition.lights[3].position = glm::vec4(0.0f, -0.9f, 0.5f, 0.0f);
   uboComposition.lights[3].color = glm::vec3(1.0f, 1.0f, 0.0f);
   uboComposition.lights[3].radius = 2.0f;
   // Green
   uboComposition.lights[4].position = glm::vec4(0.0f, -0.5f, 0.0f, 0.0f);
   uboComposition.lights[4].color = glm::vec3(0.0f, 1.0f, 0.2f);
   uboComposition.lights[4].radius = 5.0f;
   // Yellow
   uboComposition.lights[5].position = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
   uboComposition.lights[5].color = glm::vec3(1.0f, 0.7f, 0.3f);
   uboComposition.lights[5].radius = 25.0f;

   uboComposition.lights[0].position.x = sin(glm::radians(360.0f * timer)) * 5.0f;
   uboComposition.lights[0].position.z = cos(glm::radians(360.0f * timer)) * 5.0f;

   uboComposition.lights[1].position.x = -4.0f + sin(glm::radians(360.0f * timer) + 45.0f) * 2.0f;
   uboComposition.lights[1].position.z = 0.0f + cos(glm::radians(360.0f * timer) + 45.0f) * 2.0f;

   uboComposition.lights[2].position.x = 4.0f + sin(glm::radians(360.0f * timer)) * 2.0f;
   uboComposition.lights[2].position.z = 0.0f + cos(glm::radians(360.0f * timer)) * 2.0f;

   uboComposition.lights[4].position.x = 0.0f + sin(glm::radians(360.0f * timer + 90.0f)) * 5.0f;
   uboComposition.lights[4].position.z = 0.0f - cos(glm::radians(360.0f * timer + 45.0f)) * 5.0f;

   uboComposition.lights[5].position.x = 0.0f + sin(glm::radians(-360.0f * timer + 135.0f)) * 10.0f;
   uboComposition.lights[5].position.z = 0.0f - cos(glm::radians(-360.0f * timer - 45.0f)) * 10.0f;

   // Current view position
   uboComposition.viewPos =
      glm::vec4(m_camera->GetPosition(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

   uboComposition.debugDisplayTarget = m_debugDisplayTarget;

   m_compositionBuffer.CopyData(&uboComposition);
   // memcpy(uniformBuffers.composition.mapped, &uboComposition, sizeof(uboComposition));
}

void
DeferedPipeline::Initialize()
{
   m_camera = std::make_unique< scene::PerspectiveCamera >(70.0f, 16.0f / 9.0f, 0.1f, 500.0f);
   PrepareOffscreenFramebuffer();
   PrepareUniformBuffers();

   SetupDescriptorSetLayout();
   PreparePipelines();
   SetupDescriptorPool();
   SetupDescriptorSet();

   BuildCommandBuffers();
   BuildDeferredCommandBuffer();
}

void
DeferedPipeline::PrepareOffscreenFramebuffer()
{
   m_frameBuffer.Create(2048, 2048);
}

void
DeferedPipeline::PrepareUniformBuffers()
{
   // Offscreen vertex shader
   m_offscreenBuffer = Buffer::CreateBuffer(
      sizeof(uboOffscreenVS), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
   // &uniformBuffers.offscreen

   // Deferred fragment shader
   // &uniformBuffers.composition,
   m_compositionBuffer = Buffer::CreateBuffer(
      sizeof(uboComposition), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


   // Map persistent
   m_compositionBuffer.Map();
   m_offscreenBuffer.Map();
   // uniformBuffers.offscreen.map());
   // VK_CHECK(uniformBuffers.composition.map());

   // Setup instanced model positions
   uboOffscreenVS.instancePos[0] = glm::vec4(0.0f);
   uboOffscreenVS.instancePos[1] = glm::vec4(-4.0f, 0.0, -4.0f, 0.0f);
   uboOffscreenVS.instancePos[2] = glm::vec4(4.0f, 0.0, -4.0f, 0.0f);

   // Update
   UpdateUniformBufferOffscreen();
   UpdateUniformBufferComposition();
}

void
DeferedPipeline::SetupDescriptorSetLayout()
{
}

void
DeferedPipeline::PreparePipelines()
{
}

void
DeferedPipeline::SetupDescriptorPool()
{
}

void
DeferedPipeline::SetupDescriptorSet()
{
}

void
DeferedPipeline::BuildCommandBuffers()
{
}

void
DeferedPipeline::BuildDeferredCommandBuffer()
{
}


} // namespace shady::render::vulkan
