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
   EndCommandRecording();

   static void
   EndImageAcquire(VkResult result, uint32_t imageIndex);

   static void
   EndOffscreenSubmit();

   static void
   EndSceneSubmit();

   static void
   EndPresent(VkResult result);

   static void
   EndFrame();

   static void
   ResetGpuTimestamps(VkCommandBuffer commandBuffer);

   static void
   WriteGpuTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                     TimestampQuery query);
};

} // namespace shady::render
