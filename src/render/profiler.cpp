#include "profiler.hpp"

#include "common.hpp"
#include "utils/assert.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <deque>
#include <imgui.h>
#include <optional>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>

namespace shady::render {

namespace {

using FrameClock = std::chrono::steady_clock;

constexpr uint32_t TIMESTAMP_QUERY_COUNT = static_cast< uint32_t >(TimestampQuery::QueryCount);
constexpr uint32_t CPU_SAMPLE_WINDOW = 60;
constexpr size_t PACING_SAMPLE_WINDOW = 240;

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

struct FramePacingSample
{
   std::optional< float > frameIntervalMs;
   std::optional< float > presentIntervalMs;
   float queueIdleMs = 0.0f;
   std::optional< float > gpuFrameMs;
};

class TimingSampleWindow
{
 public:
   void
   Push(float sample)
   {
      samples_.push_back(sample);
      if (samples_.size() > PACING_SAMPLE_WINDOW)
      {
         samples_.pop_front();
      }
   }

   [[nodiscard]] TimingPercentiles
   Percentiles() const
   {
      TimingPercentiles result{};
      if (samples_.empty())
      {
         return result;
      }

      std::vector< float > sorted(samples_.begin(), samples_.end());
      std::sort(sorted.begin(), sorted.end());

      const auto percentile = [&sorted](float value) {
         const float position = value * static_cast< float >(sorted.size() - 1);
         const auto lower = static_cast< size_t >(std::floor(position));
         const auto upper = static_cast< size_t >(std::ceil(position));
         const float weight = position - static_cast< float >(lower);
         return sorted[lower] + ((sorted[upper] - sorted[lower]) * weight);
      };

      result.p50 = percentile(0.50f);
      result.p95 = percentile(0.95f);
      result.p99 = percentile(0.99f);
      result.maximum = sorted.back();
      return result;
   }

   [[nodiscard]] uint32_t
   Size() const
   {
      return static_cast< uint32_t >(samples_.size());
   }

 private:
   std::deque< float > samples_;
};

struct CurrentFrame
{
   FrameClock::time_point frameStart = {};
   FrameClock::time_point commandRecordEnd = {};
   FrameClock::time_point imageAcquireEnd = {};
   FrameClock::time_point offscreenSubmitEnd = {};
   FrameClock::time_point sceneSubmitEnd = {};
   FrameClock::time_point presentEnd = {};
   std::optional< float > frameIntervalMs;
   std::optional< float > presentIntervalMs;
   FrameDiagnostics gpuDiagnostics = {};
   uint32_t currentFrameIndex = 0;
   uint32_t acquiredImageIndex = 0;
   VkResult acquireResult = VK_SUCCESS;
   VkResult presentResult = VK_SUCCESS;
};

struct ProfilerState
{
   VkQueryPool timestampQueryPool = VK_NULL_HANDLE;
   float timestampPeriod = 0.0f;
   uint32_t timestampValidBits = 0;
   bool timestampResultsReady = false;

   FrameClock::time_point fpsPreviousTick = FrameClock::now();
   FrameClock::duration fpsInterval = {};
   uint32_t fpsFrames = 0;
   int32_t fps = 0;

   FrameClock::time_point uniformUpdateStart = {};
   FrameClock::time_point guiUploadStart = {};
   float uniformUpdateMs = 0.0f;
   float guiUploadMs = 0.0f;

   std::optional< FrameClock::time_point > previousFrameStart;
   std::optional< FrameClock::time_point > previousPresentReturn;
   CurrentFrame currentFrame = {};

   FrameDiagnostics totals = {};
   FrameDiagnostics published = {};
   TimingSampleWindow frameIntervals;
   TimingSampleWindow presentIntervals;
   TimingSampleWindow queueIdles;
   TimingSampleWindow gpuFrames;
};

ProfilerState&
State()
{
   static ProfilerState state;
   return state;
}

float
ElapsedMilliseconds(FrameClock::time_point start, FrameClock::time_point end)
{
   return std::chrono::duration< float, std::milli >(end - start).count();
}

uint32_t
TimestampQueryIndex(TimestampQuery query)
{
   return static_cast< uint32_t >(query);
}

uint64_t
TimestampDelta(uint64_t start, uint64_t end, uint32_t validBits)
{
   if (validBits == 64)
   {
      return end - start;
   }

   const uint64_t mask = (uint64_t{1} << validBits) - 1;
   return (end - start) & mask;
}

float
TimestampMilliseconds(uint64_t start, uint64_t end, uint32_t validBits, float timestampPeriod)
{
   constexpr float NANOSECONDS_PER_MILLISECOND = 1'000'000.0f;
   const auto ticks = TimestampDelta(start, end, validBits);
   return static_cast< float >(ticks) * timestampPeriod / NANOSECONDS_PER_MILLISECOND;
}

FrameDiagnostics
ReadGpuDiagnostics()
{
   auto& state = State();
   FrameDiagnostics diagnostics{};
   if (!state.timestampResultsReady)
   {
      return diagnostics;
   }

   std::array< uint64_t, TIMESTAMP_QUERY_COUNT > timestamps{};
   const auto result = vkGetQueryPoolResults(
      Data::vk_device, state.timestampQueryPool, 0, TIMESTAMP_QUERY_COUNT, sizeof(timestamps),
      timestamps.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);

   if (result == VK_NOT_READY)
   {
      return diagnostics;
   }

   VK_CHECK(result, "failed to read GPU timestamp queries!");

   const auto milliseconds = [&state, &timestamps](TimestampQuery start, TimestampQuery end) {
      return TimestampMilliseconds(timestamps[TimestampQueryIndex(start)],
                                   timestamps[TimestampQueryIndex(end)], state.timestampValidBits,
                                   state.timestampPeriod);
   };

   diagnostics.gpuFrameMs = milliseconds(TimestampQuery::FrameStart, TimestampQuery::FrameEnd);
   diagnostics.gpuOffscreenMs =
      milliseconds(TimestampQuery::OffscreenStart, TimestampQuery::OffscreenEnd);
   diagnostics.gpuShadowMs = milliseconds(TimestampQuery::ShadowStart, TimestampQuery::ShadowEnd);
   diagnostics.gpuGBufferMs = milliseconds(TimestampQuery::ShadowEnd, TimestampQuery::GBufferEnd);
   diagnostics.gpuBarrierMs =
      milliseconds(TimestampQuery::OffscreenEnd, TimestampQuery::CompositionStart);
   diagnostics.gpuCompositionMs =
      milliseconds(TimestampQuery::CompositionStart, TimestampQuery::CompositionEnd);
   diagnostics.gpuImGuiMs = milliseconds(TimestampQuery::ImGuiStart, TimestampQuery::ImGuiEnd);
   diagnostics.gpuSampleCount = 1;
   return diagnostics;
}

void
PublishFrameDiagnostics(const FrameDiagnostics& frame, const FramePacingSample& pacing)
{
   auto& state = State();

   state.published.currentFrameIndex = frame.currentFrameIndex;
   state.published.acquiredImageIndex = frame.acquiredImageIndex;
   state.published.acquireResult = frame.acquireResult;
   state.published.presentResult = frame.presentResult;

   if (pacing.frameIntervalMs)
   {
      state.frameIntervals.Push(*pacing.frameIntervalMs);
   }
   if (pacing.presentIntervalMs)
   {
      state.presentIntervals.Push(*pacing.presentIntervalMs);
   }
   state.queueIdles.Push(pacing.queueIdleMs);
   if (pacing.gpuFrameMs)
   {
      state.gpuFrames.Push(*pacing.gpuFrameMs);
   }

   state.totals.totalMs += frame.totalMs;
   state.totals.fenceWaitMs += frame.fenceWaitMs;
   state.totals.imageAcquireMs += frame.imageAcquireMs;
   state.totals.fenceResetMs += frame.fenceResetMs;
   state.totals.guiUploadMs += frame.guiUploadMs;
   state.totals.commandRecordMs += frame.commandRecordMs;
   state.totals.uniformUpdateMs += frame.uniformUpdateMs;
   state.totals.queueSubmitMs += frame.queueSubmitMs;
   state.totals.presentMs += frame.presentMs;
   state.totals.queueIdleMs += frame.queueIdleMs;
   if (frame.gpuSampleCount > 0)
   {
      state.totals.gpuFrameMs += frame.gpuFrameMs;
      state.totals.gpuOffscreenMs += frame.gpuOffscreenMs;
      state.totals.gpuShadowMs += frame.gpuShadowMs;
      state.totals.gpuGBufferMs += frame.gpuGBufferMs;
      state.totals.gpuBarrierMs += frame.gpuBarrierMs;
      state.totals.gpuCompositionMs += frame.gpuCompositionMs;
      state.totals.gpuImGuiMs += frame.gpuImGuiMs;
      ++state.totals.gpuSampleCount;
   }
   ++state.totals.sampleCount;

   if (state.totals.sampleCount < CPU_SAMPLE_WINDOW)
   {
      return;
   }

   const auto sampleCount = static_cast< float >(state.totals.sampleCount);
   state.published = {
      .totalMs = state.totals.totalMs / sampleCount,
      .fenceWaitMs = state.totals.fenceWaitMs / sampleCount,
      .imageAcquireMs = state.totals.imageAcquireMs / sampleCount,
      .fenceResetMs = state.totals.fenceResetMs / sampleCount,
      .guiUploadMs = state.totals.guiUploadMs / sampleCount,
      .commandRecordMs = state.totals.commandRecordMs / sampleCount,
      .uniformUpdateMs = state.totals.uniformUpdateMs / sampleCount,
      .queueSubmitMs = state.totals.queueSubmitMs / sampleCount,
      .presentMs = state.totals.presentMs / sampleCount,
      .queueIdleMs = state.totals.queueIdleMs / sampleCount,
      .sampleCount = state.totals.sampleCount,
      .currentFrameIndex = frame.currentFrameIndex,
      .acquiredImageIndex = frame.acquiredImageIndex,
      .acquireResult = frame.acquireResult,
      .presentResult = frame.presentResult,
   };

   if (state.totals.gpuSampleCount > 0)
   {
      const auto gpuSampleCount = static_cast< float >(state.totals.gpuSampleCount);
      state.published.gpuFrameMs = state.totals.gpuFrameMs / gpuSampleCount;
      state.published.gpuOffscreenMs = state.totals.gpuOffscreenMs / gpuSampleCount;
      state.published.gpuShadowMs = state.totals.gpuShadowMs / gpuSampleCount;
      state.published.gpuGBufferMs = state.totals.gpuGBufferMs / gpuSampleCount;
      state.published.gpuBarrierMs = state.totals.gpuBarrierMs / gpuSampleCount;
      state.published.gpuCompositionMs = state.totals.gpuCompositionMs / gpuSampleCount;
      state.published.gpuImGuiMs = state.totals.gpuImGuiMs / gpuSampleCount;
      state.published.gpuSampleCount = state.totals.gpuSampleCount;
   }

   state.published.frameInterval = state.frameIntervals.Percentiles();
   state.published.presentInterval = state.presentIntervals.Percentiles();
   state.published.queueIdle = state.queueIdles.Percentiles();
   state.published.gpuFrame = state.gpuFrames.Percentiles();
   state.published.pacingSampleCount = state.frameIntervals.Size();

   state.totals = {};
}

} // namespace

void
Profiler::Initialize(uint32_t graphicsQueueFamilyIndex)
{
   auto& state = State();

   VkPhysicalDeviceProperties properties{};
   vkGetPhysicalDeviceProperties(Data::vk_physicalDevice, &properties);
   state.timestampPeriod = properties.limits.timestampPeriod;

   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties(Data::vk_physicalDevice, &queueFamilyCount, nullptr);

   std::vector< VkQueueFamilyProperties > queueFamilies(queueFamilyCount);
   vkGetPhysicalDeviceQueueFamilyProperties(Data::vk_physicalDevice, &queueFamilyCount,
                                            queueFamilies.data());

   utils::Assert(graphicsQueueFamilyIndex < queueFamilies.size(),
                 "graphics queue family index is out of range!");
   state.timestampValidBits = queueFamilies[graphicsQueueFamilyIndex].timestampValidBits;
   utils::Assert(state.timestampValidBits > 0,
                 "graphics queue does not support timestamp queries!");

   VkQueryPoolCreateInfo queryPoolInfo{};
   queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
   queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
   queryPoolInfo.queryCount = TIMESTAMP_QUERY_COUNT;

   VK_CHECK(vkCreateQueryPool(Data::vk_device, &queryPoolInfo, nullptr, &state.timestampQueryPool),
            "failed to create timestamp query pool!");
}

void
Profiler::UpdateFps()
{
   auto& state = State();
   const auto now = FrameClock::now();
   state.fpsInterval += now - state.fpsPreviousTick;
   state.fpsPreviousTick = now;
   ++state.fpsFrames;

   if (state.fpsInterval < std::chrono::seconds{1})
   {
      return;
   }

   const auto elapsedSeconds = std::chrono::duration< double >(state.fpsInterval).count();
   state.fps = static_cast< int32_t >(static_cast< double >(state.fpsFrames) / elapsedSeconds);
   state.fpsFrames = 0;
   state.fpsInterval = {};
}

int32_t
Profiler::GetFps()
{
   return State().fps;
}

void
Profiler::DrawDebugUi(float valueColumn)
{
   const auto& diagnostics = State().published;

   ImGui::TextUnformatted("FPS");
   ImGui::SameLine(valueColumn);
   ImGui::Text("%d", GetFps());

   if (diagnostics.sampleCount == 0)
   {
      return;
   }

   ImGui::Separator();
   ImGui::Text("Frame diagnostics (%u-frame CPU average)", diagnostics.sampleCount);

   const auto timingValueColumn = ImGui::GetCursorPosX()
                                  + ImGui::CalcTextSize("Command recording").x
                                  + ImGui::GetStyle().ItemSpacing.x;
   const auto timingRow = [timingValueColumn](const char* label, float milliseconds) {
      ImGui::TextUnformatted(label);
      ImGui::SameLine(timingValueColumn);
      ImGui::Text("%.3f ms", static_cast< double >(milliseconds));
   };

   timingRow("Total Draw", diagnostics.totalMs);
   timingRow("Fence wait", diagnostics.fenceWaitMs);
   timingRow("Image acquire", diagnostics.imageAcquireMs);
   timingRow("Fence reset", diagnostics.fenceResetMs);
   timingRow("GUI upload", diagnostics.guiUploadMs);
   timingRow("Command recording", diagnostics.commandRecordMs);
   timingRow("Uniform updates", diagnostics.uniformUpdateMs);
   timingRow("Queue submit", diagnostics.queueSubmitMs);
   timingRow("Present", diagnostics.presentMs);
   timingRow("Queue idle", diagnostics.queueIdleMs);

   if (diagnostics.pacingSampleCount > 0)
   {
      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();
      ImGui::Text("Frame pacing (%u-sample rolling window, ms)", diagnostics.pacingSampleCount);

      const auto pacingValueColumn = ImGui::GetCursorPosX()
                                     + ImGui::CalcTextSize("Present interval").x
                                     + ImGui::GetStyle().ItemSpacing.x;
      const auto pacingColumnWidth =
         ImGui::CalcTextSize("999.999").x + (ImGui::GetStyle().ItemSpacing.x * 2.0f);
      const auto pacingHeader = [pacingValueColumn, pacingColumnWidth]() {
         ImGui::TextUnformatted("Metric");
         ImGui::SameLine(pacingValueColumn);
         ImGui::TextUnformatted("p50");
         ImGui::SameLine(pacingValueColumn + pacingColumnWidth);
         ImGui::TextUnformatted("p95");
         ImGui::SameLine(pacingValueColumn + (pacingColumnWidth * 2.0f));
         ImGui::TextUnformatted("p99");
         ImGui::SameLine(pacingValueColumn + (pacingColumnWidth * 3.0f));
         ImGui::TextUnformatted("max");
      };
      const auto pacingRow = [pacingValueColumn, pacingColumnWidth](
                                const char* label, const TimingPercentiles& percentiles) {
         ImGui::TextUnformatted(label);
         ImGui::SameLine(pacingValueColumn);
         ImGui::Text("%.3f", static_cast< double >(percentiles.p50));
         ImGui::SameLine(pacingValueColumn + pacingColumnWidth);
         ImGui::Text("%.3f", static_cast< double >(percentiles.p95));
         ImGui::SameLine(pacingValueColumn + (pacingColumnWidth * 2.0f));
         ImGui::Text("%.3f", static_cast< double >(percentiles.p99));
         ImGui::SameLine(pacingValueColumn + (pacingColumnWidth * 3.0f));
         ImGui::Text("%.3f", static_cast< double >(percentiles.maximum));
      };

      pacingHeader();
      pacingRow("Frame interval", diagnostics.frameInterval);
      pacingRow("Present interval", diagnostics.presentInterval);
      pacingRow("Queue idle", diagnostics.queueIdle);
      pacingRow("GPU frame", diagnostics.gpuFrame);

      ImGui::Spacing();
      ImGui::Text("Frame slot: %u    Image index: %u", diagnostics.currentFrameIndex,
                  diagnostics.acquiredImageIndex);
      ImGui::Text("Acquire: %s",
                  string_VkResult(static_cast< VkResult >(diagnostics.acquireResult)));
      ImGui::Text("Present: %s",
                  string_VkResult(static_cast< VkResult >(diagnostics.presentResult)));
   }

   if (diagnostics.gpuSampleCount > 0)
   {
      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();
      ImGui::Text("GPU diagnostics (%u-frame average)", diagnostics.gpuSampleCount);
      timingRow("GPU frame", diagnostics.gpuFrameMs);
      timingRow("GPU offscreen", diagnostics.gpuOffscreenMs);
      timingRow("GPU shadow", diagnostics.gpuShadowMs);
      timingRow("GPU G-buffer", diagnostics.gpuGBufferMs);
      timingRow("GPU queue gap", diagnostics.gpuBarrierMs);
      timingRow("GPU composition", diagnostics.gpuCompositionMs);
      timingRow("GPU ImGui", diagnostics.gpuImGuiMs);
   }
}

void
Profiler::BeginUniformUpdate()
{
   State().uniformUpdateStart = FrameClock::now();
}

void
Profiler::EndUniformUpdate()
{
   auto& state = State();
   state.uniformUpdateMs = ElapsedMilliseconds(state.uniformUpdateStart, FrameClock::now());
}

void
Profiler::BeginGuiUpload()
{
   State().guiUploadStart = FrameClock::now();
}

void
Profiler::EndGuiUpload()
{
   auto& state = State();
   state.guiUploadMs = ElapsedMilliseconds(state.guiUploadStart, FrameClock::now());
}

void
Profiler::BeginFrame(uint32_t currentFrameIndex)
{
   auto& state = State();
   const auto frameStart = FrameClock::now();

   state.currentFrame = {};
   state.currentFrame.frameStart = frameStart;
   state.currentFrame.currentFrameIndex = currentFrameIndex;
   if (state.previousFrameStart)
   {
      state.currentFrame.frameIntervalMs =
         ElapsedMilliseconds(*state.previousFrameStart, frameStart);
   }
   state.previousFrameStart = frameStart;
   state.currentFrame.gpuDiagnostics = ReadGpuDiagnostics();
}

void
Profiler::EndCommandRecording()
{
   State().currentFrame.commandRecordEnd = FrameClock::now();
}

void
Profiler::EndImageAcquire(VkResult result, uint32_t imageIndex)
{
   auto& frame = State().currentFrame;
   frame.imageAcquireEnd = FrameClock::now();
   frame.acquireResult = result;
   frame.acquiredImageIndex = imageIndex;
}

void
Profiler::EndOffscreenSubmit()
{
   State().currentFrame.offscreenSubmitEnd = FrameClock::now();
}

void
Profiler::EndSceneSubmit()
{
   State().currentFrame.sceneSubmitEnd = FrameClock::now();
}

void
Profiler::EndPresent(VkResult result)
{
   auto& state = State();
   auto& frame = state.currentFrame;
   frame.presentEnd = FrameClock::now();
   frame.presentResult = result;
   if (state.previousPresentReturn)
   {
      frame.presentIntervalMs = ElapsedMilliseconds(*state.previousPresentReturn, frame.presentEnd);
   }
   state.previousPresentReturn = frame.presentEnd;
}

void
Profiler::EndFrame()
{
   auto& state = State();
   auto& frame = state.currentFrame;
   const auto frameEnd = FrameClock::now();
   state.timestampResultsReady = true;

   const float queueIdleMs = ElapsedMilliseconds(frame.presentEnd, frameEnd);
   const auto& gpu = frame.gpuDiagnostics;
   PublishFrameDiagnostics(
      {
         .totalMs = ElapsedMilliseconds(frame.frameStart, frameEnd),
         .imageAcquireMs = ElapsedMilliseconds(frame.commandRecordEnd, frame.imageAcquireEnd),
         .guiUploadMs = state.guiUploadMs,
         .commandRecordMs = ElapsedMilliseconds(frame.frameStart, frame.commandRecordEnd),
         .uniformUpdateMs = state.uniformUpdateMs,
         .queueSubmitMs = ElapsedMilliseconds(frame.imageAcquireEnd, frame.offscreenSubmitEnd)
                          + ElapsedMilliseconds(frame.offscreenSubmitEnd, frame.sceneSubmitEnd),
         .presentMs = ElapsedMilliseconds(frame.sceneSubmitEnd, frame.presentEnd),
         .queueIdleMs = queueIdleMs,
         .gpuFrameMs = gpu.gpuFrameMs,
         .gpuOffscreenMs = gpu.gpuOffscreenMs,
         .gpuShadowMs = gpu.gpuShadowMs,
         .gpuGBufferMs = gpu.gpuGBufferMs,
         .gpuBarrierMs = gpu.gpuBarrierMs,
         .gpuCompositionMs = gpu.gpuCompositionMs,
         .gpuImGuiMs = gpu.gpuImGuiMs,
         .gpuSampleCount = gpu.gpuSampleCount,
         .currentFrameIndex = frame.currentFrameIndex,
         .acquiredImageIndex = frame.acquiredImageIndex,
         .acquireResult = static_cast< int32_t >(frame.acquireResult),
         .presentResult = static_cast< int32_t >(frame.presentResult),
      },
      {
         .frameIntervalMs = frame.frameIntervalMs,
         .presentIntervalMs = frame.presentIntervalMs,
         .queueIdleMs = queueIdleMs,
         .gpuFrameMs =
            gpu.gpuSampleCount > 0 ? std::optional< float >(gpu.gpuFrameMs) : std::nullopt,
      });
}

void
Profiler::ResetGpuTimestamps(VkCommandBuffer commandBuffer)
{
   vkCmdResetQueryPool(commandBuffer, State().timestampQueryPool, 0, TIMESTAMP_QUERY_COUNT);
}

void
Profiler::WriteGpuTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                            TimestampQuery query)
{
   vkCmdWriteTimestamp(commandBuffer, pipelineStage, State().timestampQueryPool,
                       TimestampQueryIndex(query));
}

} // namespace shady::render
