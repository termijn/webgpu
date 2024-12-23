#pragma once

#include <glm/glm.hpp>

#include <webgpu/webgpu.h>

#include "gpu.h"
#include "rendertarget.h"
#include "vertexbuffer.h"
#include "uniforms.h"
#include "renderable.h"
#include "uniformsdata.h"
#include "rendertarget.h"

class ShadowPass
{
public:
    ShadowPass(Gpu& gpu, DepthTarget& target);
    virtual ~ShadowPass();
    
    struct RenderParams
    {
        glm::mat4 view;
        glm::mat4 projection;
    };

    void renderPre  (const RenderParams& params);
    void render     (const std::vector<const Renderable*>& renderables);

private:
    Gpu&                m_gpu;
    DepthTarget&        m_renderTarget;

    FrameDataShadow             m_frameData;
    Uniforms<FrameDataShadow>   m_uniformsFrame;

    ModelDataShadow              m_modelData;
    Uniforms<ModelDataShadow>    m_uniformsModel;

    WGPUPipelineLayout      m_layout {};
    std::array<WGPUBindGroupLayout, 2> m_bindGroupLayouts {};

    WGPUDepthStencilState   m_depthStencilState {};
    WGPURenderPipeline      m_pipeline {};

    std::array<WGPUBindGroup, 2> m_bindGroups { nullptr, nullptr };

    void createPipeline     ();
    void createLayout       (WGPURenderPipelineDescriptor& pipeline);
    void createBindings     ();
    void drawCommands       (WGPURenderPassEncoder ShadowPass, const std::vector<const Renderable*>& renderables);
};
