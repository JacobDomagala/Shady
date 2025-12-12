#pragma once

#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <string>

namespace shady::render {

enum class TextureType : std::uint8_t
{
   DIFFUSE_MAP = 0,
   METALLIC_ROUGHNESS_MAP = 1,
   NORMAL_MAP = 2,
   CUBE_MAP = 3
};

struct UniformBufferObject
{
   glm::mat4 proj = {};
   glm::mat4 view = {};
   glm::mat4 lightView = {};
};

struct DebugData
{
   uint32_t displayDebugTarget = 0;
   int32_t pcfShadow = 1;
   float ambientLight = 0.1f;
   float shadowFactor = 0.1f;
};

enum class TimestampQuery : uint32_t
{
   FrameStart,
   OffscreenStart,
   ShadowStart,
   ShadowEnd,
   GBufferEnd,
   OffscreenEnd,
   CompositionStart,
   CompositionEnd,
   ImGuiStart,
   ImGuiEnd,
   FrameEnd,
   QueryCount
};

constexpr uint32_t TIMESTAMP_QUERY_COUNT = static_cast< uint32_t >(TimestampQuery::QueryCount);

constexpr uint32_t
TimestampQueryIndex(TimestampQuery query)
{
   return static_cast< uint32_t >(query);
}

struct TimingPercentiles
{
   float p50 = 0.0f;
   float p95 = 0.0f;
   float p99 = 0.0f;
   float maximum = 0.0f;
};

struct FrameDiagnostics
{
   float totalMs = 0.0f;
   float fenceWaitMs = 0.0f;
   float imageAcquireMs = 0.0f;
   float fenceResetMs = 0.0f;
   float guiUploadMs = 0.0f;
   float commandRecordMs = 0.0f;
   float uniformUpdateMs = 0.0f;
   float queueSubmitMs = 0.0f;
   float presentMs = 0.0f;
   float queueIdleMs = 0.0f;
   float gpuFrameMs = 0.0f;
   float gpuOffscreenMs = 0.0f;
   float gpuShadowMs = 0.0f;
   float gpuGBufferMs = 0.0f;
   float gpuBarrierMs = 0.0f;
   float gpuCompositionMs = 0.0f;
   float gpuImGuiMs = 0.0f;
   TimingPercentiles frameInterval = {};
   TimingPercentiles presentInterval = {};
   TimingPercentiles queueIdle = {};
   TimingPercentiles gpuFrame = {};
   uint32_t sampleCount = 0;
   uint32_t gpuSampleCount = 0;
   uint32_t pacingSampleCount = 0;
   uint32_t currentFrameIndex = 0;
   uint32_t acquiredImageIndex = 0;
   int32_t acquireResult = 0;
   int32_t presentResult = 0;
};

struct PerInstanceBuffer
{
   glm::mat4 model = {};
   glm::ivec4 textures = glm::ivec4(-1);
   glm::vec4 baseColorFactor = glm::vec4(1.0F);
   glm::vec4 materialFactors = glm::vec4(1.0F, 1.0F, 1.0F, 0.0F);
};

// Base color, metallic-roughness, normal.
using TextureMaps = std::array< std::string, 3 >;

struct MaterialData
{
   TextureMaps textures = {};
   glm::vec4 baseColorFactor = glm::vec4(1.0F);
   float metallicFactor = 1.0F;
   float roughnessFactor = 1.0F;
   float normalScale = 1.0F;
};

} // namespace shady::render
