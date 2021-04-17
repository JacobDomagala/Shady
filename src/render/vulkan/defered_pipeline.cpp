#include "defered_pipeline.hpp"
#include "vulkan_common.hpp"

namespace shady::render::vulkan {

void
DeferedPipeline::Initialize()
{
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

void DeferedPipeline::PrepareUniformBuffers(){

}

void DeferedPipeline::SetupDescriptorSetLayout(){

}

void DeferedPipeline::PreparePipelines(){

}

void DeferedPipeline::SetupDescriptorPool(){

}

void DeferedPipeline::SetupDescriptorSet(){

}

void DeferedPipeline::BuildCommandBuffers(){

}

void DeferedPipeline::BuildDeferredCommandBuffer(){

}


} // namespace shady::render::vulkan
