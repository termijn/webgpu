#pragma once

#include <webgpu/webgpu.h>
#include <array>

class RenderPipeline
{
public:

private:
    WGPUPipelineLayout                  m_layout {};
    std::array<WGPUBindGroupLayout, 2>  m_bindGroupLayouts {};

    WGPUDepthStencilState   m_depthStencilState {};
    WGPURenderPipeline      m_pipeline {};
};