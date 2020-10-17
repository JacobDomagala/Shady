#include "perspective_camera.hpp"

#include <glm/gtx/transform.hpp>

namespace shady::render {

PerspectiveCamera::PerspectiveCamera(const glm::mat4& projection) : Camera(projection)
{
}

PerspectiveCamera::PerspectiveCamera(float fieldOfView, float aspectRation, float near, float far)
{
   m_projectionMat = glm::perspective(fieldOfView, aspectRation, near, far);
}

} // namespace shady::render