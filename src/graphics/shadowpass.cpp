#include "shadowpass.h"
#include "renderpasshelpers.h"

ShadowPass::ShadowPass(Gpu & gpu, DepthTarget & target)
    : m_gpu           (gpu)
    , m_renderTarget  (target)
    , m_uniformsFrame (gpu)
    , m_uniformsModel (gpu)
{
    createPipeline();
}

ShadowPass::~ShadowPass()
{
    // TODO: if nothing is rendered, this throws an exception
    for (WGPUBindGroup& group : m_bindGroups)
        wgpuBindGroupRelease(group);

    wgpuPipelineLayoutRelease(m_layout);

    for (WGPUBindGroupLayout& layout : m_bindGroupLayouts)
        wgpuBindGroupLayoutRelease(layout);

    wgpuRenderPipelineRelease(m_pipeline);
}

void ShadowPass::render(const std::vector<const Renderable*>& renderables)
{
    m_renderTarget.setSize({ 2048, 2048 } );
    WGPUCommandEncoder& encoder = m_gpu.m_currentCommandEncoder;

    WGPURenderPassDepthStencilAttachment depthStencilAttachment{};
    depthStencilAttachment.view             =  m_renderTarget.getDepthTextureView();
    depthStencilAttachment.depthClearValue  = 1.0f;
    depthStencilAttachment.depthLoadOp      = WGPULoadOp_Clear;
    depthStencilAttachment.depthStoreOp     = WGPUStoreOp_Store;
    depthStencilAttachment.depthReadOnly    = false;
    depthStencilAttachment.stencilClearValue = 0;
    depthStencilAttachment.stencilLoadOp     = WGPULoadOp_Clear;
    depthStencilAttachment.stencilStoreOp    = WGPUStoreOp_Store;
    depthStencilAttachment.stencilReadOnly   = true;

    #ifdef WEBGPU_BACKEND_DAWN
     depthStencilAttachment.stencilLoadOp        = WGPULoadOp_Undefined;
     depthStencilAttachment.stencilStoreOp       = WGPUStoreOp_Undefined;
    // depthStencilAttachment.depthClearValue      = std::numeric_limits<float>::quiet_NaN();
    #else 
    depthStencilAttachment.stencilLoadOp        = WGPULoadOp_Clear;
    depthStencilAttachment.stencilStoreOp       = WGPUStoreOp_Store;
    #endif

    WGPURenderPassDescriptor renderPassDesc{};
    renderPassDesc.nextInChain              = nullptr;
    renderPassDesc.label                    = "shadow render pass";
    renderPassDesc.depthStencilAttachment   = &depthStencilAttachment;

    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

    glm::vec2 size = m_renderTarget.getSize();
    wgpuRenderPassEncoderSetViewport(renderPass, 0, 0, size.x, size.y, 0.0, 1.0);
    wgpuRenderPassEncoderSetPipeline(renderPass, m_pipeline);

    drawCommands(renderPass, renderables);

    wgpuRenderPassEncoderEnd    (renderPass);
    wgpuRenderPassEncoderRelease(renderPass);
}

void ShadowPass::createPipeline()
{
    std::string shaderSource = m_gpu.compileShader("./shaders/shadowpass.wgsl");

    WGPUShaderModuleDescriptor shaderDesc{};
    #ifdef WEBGPU_BACKEND_WGPU
        shaderDesc.hintCount = 0;
        shaderDesc.hints = nullptr;
    #endif

    WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;

    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    shaderCodeDesc.code = shaderSource.c_str();
    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(m_gpu.m_device, &shaderDesc);

    WGPURenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.label = "shadow pass pipeline";
    pipelineDesc.nextInChain = nullptr;

    setDefault(m_depthStencilState);
    m_depthStencilState.depthCompare          = WGPUCompareFunction_Less;
    m_depthStencilState.depthWriteEnabled     = true;
    m_depthStencilState.format                = WGPUTextureFormat_Depth32Float;
    m_depthStencilState.stencilReadMask       = 0;
    m_depthStencilState.stencilWriteMask      = 0;

    pipelineDesc.depthStencil = &m_depthStencilState;

    VertexBuffer::Layout vertexLayout {};
    pipelineDesc.vertex.bufferCount     = 1;
    pipelineDesc.vertex.buffers         = &vertexLayout.layout;
    pipelineDesc.vertex.module          = shaderModule;
    pipelineDesc.vertex.entryPoint      = "vs_main";

    pipelineDesc.primitive.topology         = WGPUPrimitiveTopology_TriangleList;
    pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipelineDesc.primitive.frontFace        = WGPUFrontFace_CCW;
    pipelineDesc.primitive.cullMode         = WGPUCullMode_None;

    createLayout(pipelineDesc);

    pipelineDesc.multisample.count  = 1;
    pipelineDesc.multisample.mask   = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;

    m_pipeline = wgpuDeviceCreateRenderPipeline(m_gpu.m_device, &pipelineDesc);

    wgpuShaderModuleRelease(shaderModule);
}

void ShadowPass::createLayout(WGPURenderPipelineDescriptor& pipeline)
{
    WGPUBindGroupLayoutEntry entryFrameUniforms{};
    fillUniformsBindGroupLayoutEntry(entryFrameUniforms, 0, sizeof(FrameDataShadow), false);
    
    WGPUBindGroupLayoutEntry entryModelUniforms{};
    fillUniformsBindGroupLayoutEntry(entryModelUniforms, 0, sizeof(ModelDataShadow), true);
    
    WGPUBindGroupLayoutDescriptor groupLayoutFrame{};
    groupLayoutFrame.label = "grouplayoutFrame - per frame data";
    groupLayoutFrame.entryCount = 1;
    groupLayoutFrame.entries = &entryFrameUniforms;
    m_bindGroupLayouts[0] = wgpuDeviceCreateBindGroupLayout(m_gpu.m_device, &groupLayoutFrame);

    WGPUBindGroupLayoutDescriptor groupLayoutModel{};
    groupLayoutModel.label = "grouplayoutModel - per model data";
    groupLayoutModel.entryCount = 1;
    groupLayoutModel.entries = &entryModelUniforms;
    m_bindGroupLayouts[1] = wgpuDeviceCreateBindGroupLayout(m_gpu.m_device, &groupLayoutModel);

    WGPUPipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = m_bindGroupLayouts.size();
    layoutDesc.bindGroupLayouts     = m_bindGroupLayouts.data();
    m_layout = wgpuDeviceCreatePipelineLayout(m_gpu.m_device, &layoutDesc);

    pipeline.layout = m_layout;
}

void ShadowPass::createBindings()
{
    for (WGPUBindGroup& group : m_bindGroups) if (group != nullptr)
        wgpuBindGroupRelease(group);

    WGPUBindGroupEntry entry0{};
    entry0.nextInChain = nullptr;
    entry0.binding = 0; 
    entry0.buffer = m_uniformsFrame.m_buffer;
    entry0.offset = 0;
    entry0.size = sizeof(FrameDataShadow);
    
    WGPUBindGroupEntry entry1{};
    entry1.nextInChain = nullptr;
    entry1.binding = 0; 
    entry1.buffer = m_uniformsModel.m_buffer;
    entry1.offset = 0;
    entry1.size = sizeof(ModelDataShadow);

    WGPUBindGroupDescriptor group0{};
    group0.nextInChain   = nullptr;
    group0.layout        = m_bindGroupLayouts[0];
    group0.entryCount    = 1;
    group0.entries       = &entry0;

    WGPUBindGroupDescriptor group1{};
    group1.nextInChain   = nullptr;
    group1.layout        = m_bindGroupLayouts[1];
    group1.entryCount    = 1;
    group1.entries       = &entry1;

    m_bindGroups[0] = wgpuDeviceCreateBindGroup(m_gpu.m_device, &group0);
    m_bindGroups[1] = wgpuDeviceCreateBindGroup(m_gpu.m_device, &group1);
}

void ShadowPass::renderPre(const RenderParams& params)
{
    m_frameData.view                = params.view;
    m_frameData.projection          = params.projection;
}

void ShadowPass::drawCommands(WGPURenderPassEncoder renderPass, const std::vector<const Renderable*>& renderables)
{
    m_uniformsFrame.setSize(1);
    m_uniformsFrame.writeChanges(0, m_frameData);

    m_uniformsModel.setSize(renderables.size());

    int index = 0;
    for (const Renderable* renderable : renderables)
    {
        ModelDataShadow data
        {
            .model = renderable->object->getSpace().toRoot,
        };
        m_uniformsModel.writeChanges(index, data);

        createBindings(); 

        uint32_t dataOffset = index * m_gpu.uniformStride(sizeof(ModelDataShadow));

        VertexBuffer& vertexBuffer = m_gpu.getResourcePool().get(renderable);

        wgpuRenderPassEncoderSetBindGroup(renderPass, 0, m_bindGroups[0], 0, nullptr);
        wgpuRenderPassEncoderSetBindGroup(renderPass, 1, m_bindGroups[1], 1, &dataOffset);

        wgpuRenderPassEncoderSetIndexBuffer (renderPass, vertexBuffer.m_indexBuffer, WGPUIndexFormat_Uint32, 0, wgpuBufferGetSize(vertexBuffer.m_indexBuffer));
        wgpuRenderPassEncoderSetVertexBuffer(renderPass,  0, vertexBuffer.m_vertexBuffer, 0, wgpuBufferGetSize(vertexBuffer.m_vertexBuffer));
        wgpuRenderPassEncoderDrawIndexed    (renderPass, vertexBuffer.m_mesh->indices().size() * 3, 1, 0, 0, 0);
        index++;
    }
}
