#pragma once

#include <glm/glm.hpp>

#include <webgpu/webgpu.h>

#include "gpu.h"
#include "rendertarget.h"
#include "vertexbuffer.h"
#include "uniforms.h"
#include "renderable.h"
#include "uniformsdata.h"

class RenderPass
{
public:
    RenderPass(Gpu& gpu, RenderTarget& renderTarget);
    virtual ~RenderPass();
    
    struct RenderParams
    {
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 worldToLight;
        glm::vec4 lightPosWorld;
    };

    void renderPre  (const RenderParams& params);
    void render     (const std::vector<const Renderable*>& renderables);

private:
    Gpu&                m_gpu;
    RenderTarget&       m_renderTarget;

    FrameData              m_frameData;
    Uniforms<FrameData>    m_uniformsFrame;

    ModelData              m_modelData;
    Uniforms<ModelData>    m_uniformsModel;

    WGPUPipelineLayout      m_layout {};
    std::array<WGPUBindGroupLayout, 2> m_bindGroupLayouts {};

    WGPUDepthStencilState   m_depthStencilState {};
    WGPURenderPipeline      m_pipeline {};

    std::array<WGPUBindGroup, 2> m_bindGroups { nullptr, nullptr };

    void createPipeline();
    void createLayout(WGPURenderPipelineDescriptor& pipeline);
    void createBindings(Texture& texture);
    void drawCommands(WGPURenderPassEncoder renderPass, const std::vector<const Renderable*>& renderables);
};
