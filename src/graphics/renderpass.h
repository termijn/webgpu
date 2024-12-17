#pragma once

#include <glm/glm.hpp>

#include "gpu.h"
#include "rendertarget.h"
#include "vertexbuffer.h"
#include "uniforms.h"
#include "renderable.h"

struct FrameData
{
    glm::mat4 view              = glm::mat4(1.0);
    glm::mat4 projection        = glm::mat4(1.0);
    glm::vec4 viewPositionWorld = glm::vec4(0.0);
};

struct ModelData
{
    glm::mat4 model         = glm::mat4(1.0);
    glm::mat4 modelInverse  = glm::mat4(1.0);
};

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
        glm::vec3 lightPosWorld;
    };

    void renderPre(const RenderParams& params);
    void render(const std::vector<const Renderable*>& renderables);

private:
    Gpu&                m_gpu;
    RenderTarget&       m_renderTarget;

    Mesh m_mesh;
    
    FrameData              m_frameData;
    Uniforms<FrameData>    m_uniformsFrame;

    ModelData              m_modelData;
    Uniforms<ModelData>    m_uniformsModel;

    WGPUPipelineLayout      m_layout {};
    std::array<WGPUBindGroupLayout, 2> m_bindGroupLayouts {};

    WGPUDepthStencilState   m_depthStencilState {};
    WGPURenderPipeline      m_pipeline {};

    std::array<WGPUBindGroup, 2> m_bindGroups {};

    VertexBuffer m_vertexBuffer;

    void createPipeline();
    void createLayout(WGPURenderPipelineDescriptor& pipeline);
    void createBindings();
    void drawCommands(WGPURenderPassEncoder encoder);
};
