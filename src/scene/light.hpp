#pragma once

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

   /*render::TextureHandleType
   GetDepthMapHandle();

   render::TextureIDType
   GetDepthMapID();*/

   const glm::mat4&
   GetLightSpaceMat() const;

   glm::vec3
   GetPosition() const;

   glm::tvec2<uint32_t>
   GetLightmapSize() const;

   void
   MoveBy(const glm::vec3& moveBy);

   void
   BindLightMap(uint32_t slot);

 private:
   uint32_t m_shadowTextureWidth = 4096;
   uint32_t m_shadowTextureHeight = 4096;
   // std::shared_ptr< render::FrameBuffer > m_shadowBuffer;

   glm::vec3 m_position = glm::vec3(0.0f);

   glm::mat4 m_projectionMatrix = glm::mat4();
   glm::mat4 m_viewMatrix = glm::mat4();
   glm::mat4 m_lightSpaceMatrix = glm::mat4();
   glm::mat4 m_biasMatrix = glm::mat4();
   glm::mat4 m_shadowMatrix = glm::mat4();
};

} // namespace shady::scene
