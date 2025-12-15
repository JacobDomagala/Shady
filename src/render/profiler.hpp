#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

namespace shady::render {

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

class Profiler
{
 public:
   static void
   Initialize(uint32_t graphicsQueueFamilyIndex);

   static void
   StartFpsCounter();

   static void
   UpdateFps();

   [[nodiscard]] static int32_t
   GetFps();

   static void
   DrawDebugUi(float valueColumn);

   static void
   BeginUniformUpdate();

   static void
   EndUniformUpdate();

   static void
   BeginGuiUpload();

   static void
   EndGuiUpload();

   static void
   BeginFrame(uint32_t currentFrameIndex);

   static void
   EndFenceWait();

   static void
   BeginImageAcquire();

   static void
   EndCommandRecording();

   static void
   EndImageAcquire(VkResult result, uint32_t imageIndex);

   static void
   BeginFenceReset();

   static void
   EndFenceReset();

   static void
   BeginOffscreenSubmit();

   static void
   EndOffscreenSubmit();

   static void
   BeginCommandRecording();

   static void
   BeginSceneSubmit();

   static void
   EndSceneSubmit();

   static void
   BeginPresent();

   static void
   EndPresent(VkResult result);

   static void
   EndFrame(bool gpuTimestampsComplete = true);

   static void
   ResetGpuTimestamps(VkCommandBuffer commandBuffer, uint32_t frameIndex);

   static void
   WriteGpuTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                     TimestampQuery query, uint32_t frameIndex);
};

} // namespace shady::render
