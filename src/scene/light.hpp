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

   [[nodiscard]] const glm::mat4&
   GetLightSpaceMat() const;

   [[nodiscard]] const glm::mat4&
   GetViewMat() const;

   [[nodiscard]] glm::vec3
   GetPosition() const;

   [[nodiscard]] glm::vec3
   GetLookAt() const;

   [[nodiscard]] glm::vec3
   GetColor() const;

   void
   SetColor(const glm::vec3& new_color);

   [[nodiscard]] glm::tvec2< uint32_t >
   GetLightmapSize() const;

   void
   MoveBy(const glm::vec3& moveBy);

 private:
   void
   UpdateViewProjection();

 private:
   uint32_t shadowTextureWidth_ = 4096;
   uint32_t shadowTextureHeight_ = 4096;
   // std::shared_ptr< render::FrameBuffer > m_shadowBuffer;

   glm::vec3 position_ = glm::vec3(0.0f);
   glm::vec3 lookAt_ = glm::vec3(0.0f);
   glm::vec3 upVec_ = glm::vec3(0.0f);
   glm::vec3 color_ = glm::vec3(1.0f);

   glm::mat4 projectionMatrix_ = glm::mat4();
   glm::mat4 viewMatrix_ = glm::mat4();
   glm::mat4 lightSpaceMatrix_ = glm::mat4();
   glm::mat4 biasMatrix_ = glm::mat4();
   glm::mat4 shadowMatrix_ = glm::mat4();
};

} // namespace shady::scene
