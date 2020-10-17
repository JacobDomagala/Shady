#pragma once

#include "render/framebuffer.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace shady::scene {

enum class LightType
{
   DIRECTIONAL_LIGHT,
   POINT_LIGHT,
   SPOTLIGHT
};

class Light
{
 public:
   Light(const glm::vec3& position, const glm::vec3& color, LightType type);

   void
   BeginRenderToLightmap();

   void
   EndRenderToLightmap();

   void
   BindLightMap();

   const glm::mat4&
   GetLightSpaceMat() const;

 private:
   int32_t m_shadowTextureWidth = 4096;
   int32_t m_shadowTextureHeight = 4096;
   std::shared_ptr< render::FrameBuffer > m_shadowBuffer;

   glm::vec3 m_color = glm::vec3(1.0f);
   glm::vec3 m_position = glm::vec3(0.0f);
   float m_intensity = 1.0f;

   glm::mat4 m_modelMatrix = glm::mat4();
   glm::mat4 m_lightSpaceMatrix = glm::mat4();
   glm::mat4 m_biasMatrix = glm::mat4();
   glm::mat4 m_shadowMatrix = glm::mat4();
};

} // namespace shady::scene