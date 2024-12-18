#include "renderpass.h"
#include <iostream>

using namespace glm;

RenderPass::RenderPass(Gpu &gpu, RenderTarget &renderTarget)
    : m_gpu           (gpu)
    , m_renderTarget  (renderTarget)
    , m_uniformsFrame (gpu, m_frameData)
    , m_uniformsModel (gpu, m_modelData)
{
    createPipeline();
}

RenderPass::~RenderPass()
{
    for (WGPUBindGroup& group : m_bindGroups)
        wgpuBindGroupRelease(group);

    wgpuPipelineLayoutRelease(m_layout);

    for (WGPUBindGroupLayout& layout : m_bindGroupLayouts)
        wgpuBindGroupLayoutRelease(layout);

    wgpuRenderPipelineRelease(m_pipeline);
}

void RenderPass::renderPre(const RenderParams& params)
{
    m_frameData.view                = params.view;
    m_frameData.projection          = params.projection;
    m_frameData.viewPositionWorld   = inverse(params.view)[3];
    m_frameData.lightPositionWorld  = params.lightPosWorld;
}

void RenderPass::render(const std::vector<const Renderable*>& renderables)
{
    WGPUCommandEncoder encoder = m_gpu.createCommandEncoder();

    WGPUTextureView targetView = m_renderTarget.getNextTextureView();

    // The attachment part of the render pass descriptor describes the target texture of the pass
    WGPURenderPassColorAttachment renderPassColorAttachment{};
    renderPassColorAttachment.view          = targetView;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp        = WGPULoadOp_Clear;
    renderPassColorAttachment.storeOp       = WGPUStoreOp_Store;
    renderPassColorAttachment.clearValue    = WGPUColor{ 0.05, 0.05, 0.05, 1.0 };

    #ifndef WEBGPU_BACKEND_WGPU
    renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    #endif

    WGPURenderPassDepthStencilAttachment depthStencilAttachment{};
    depthStencilAttachment.view             =  m_renderTarget.getDepthTextureView();
    depthStencilAttachment.depthClearValue  = 1.0f;
    depthStencilAttachment.depthLoadOp      = WGPULoadOp_Clear;
    depthStencilAttachment.depthStoreOp     = WGPUStoreOp_Store;
    depthStencilAttachment.depthReadOnly    = false;
    depthStencilAttachment.stencilClearValue    = 0;
    depthStencilAttachment.stencilLoadOp        = WGPULoadOp_Clear;
    depthStencilAttachment.stencilStoreOp       = WGPUStoreOp_Store;
    depthStencilAttachment.stencilReadOnly      = true;

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
    renderPassDesc.label                    = "render pass";
    renderPassDesc.colorAttachmentCount     = 1;
    renderPassDesc.colorAttachments         = &renderPassColorAttachment;
    renderPassDesc.depthStencilAttachment   = &depthStencilAttachment;

    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

    glm::vec2 size = m_renderTarget.getSize();
    wgpuRenderPassEncoderSetViewport(renderPass, 0, 0, size.x, size.y, 0.0, 1.0);
    wgpuRenderPassEncoderSetPipeline(renderPass, m_pipeline);

    wgpuRenderPassEncoderSetBindGroup(renderPass, 0, m_bindGroups[0], 0, nullptr);
    wgpuRenderPassEncoderSetBindGroup(renderPass, 1, m_bindGroups[1], 0, nullptr);

    drawCommands(renderPass, renderables);

    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);

    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.nextInChain = nullptr;
    cmdBufferDescriptor.label       = "Command buffer";
    WGPUCommandBuffer command       = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);

    wgpuCommandEncoderRelease(encoder);

    wgpuQueueSubmit(m_gpu.m_queue, 1, &command);
    wgpuCommandBufferRelease(command);

#if defined(WEBGPU_BACKEND_DAWN)
    wgpuDeviceTick(m_gpu.m_device);
#elif defined(WEBGPU_BACKEND_WGPU)
    wgpuDevicePoll(device, false, nullptr);
#endif
}

void RenderPass::drawCommands(WGPURenderPassEncoder encoder, const std::vector<const Renderable*>& renderables)
{
    m_uniformsFrame.writeChanges();
    m_uniformsModel.writeChanges();

    for (const Renderable* renderable : renderables)
    {
        VertexBuffer& vertexBuffer = m_gpu.getResourcePool().get(renderable);

        wgpuRenderPassEncoderSetIndexBuffer (encoder, vertexBuffer.m_indexBuffer, WGPUIndexFormat_Uint32, 0, wgpuBufferGetSize(vertexBuffer.m_indexBuffer));
        wgpuRenderPassEncoderSetVertexBuffer(encoder,  0, vertexBuffer.m_vertexBuffer, 0, wgpuBufferGetSize(vertexBuffer.m_vertexBuffer));
        wgpuRenderPassEncoderDrawIndexed    (encoder, vertexBuffer.m_mesh->indices().size() * 3, 1, 0, 0, 0);
    }
}

void setDefault(WGPUBindGroupLayoutEntry &bindingLayout) 
{
    bindingLayout.buffer.nextInChain = nullptr;
    bindingLayout.buffer.type = WGPUBufferBindingType_Undefined;
    bindingLayout.buffer.hasDynamicOffset = false;

    bindingLayout.sampler.nextInChain = nullptr;
    bindingLayout.sampler.type = WGPUSamplerBindingType_Undefined;

    bindingLayout.storageTexture.nextInChain = nullptr;
    bindingLayout.storageTexture.access = WGPUStorageTextureAccess_Undefined;
    bindingLayout.storageTexture.format = WGPUTextureFormat_Undefined;
    bindingLayout.storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

    bindingLayout.texture.nextInChain = nullptr;
    bindingLayout.texture.multisampled = false;
    bindingLayout.texture.sampleType = WGPUTextureSampleType_Undefined;
    bindingLayout.texture.viewDimension = WGPUTextureViewDimension_Undefined;
}

void setDefault(WGPUStencilFaceState &stencilFaceState)
{
    stencilFaceState.compare = WGPUCompareFunction_Always;
    stencilFaceState.failOp = WGPUStencilOperation_Keep;
    stencilFaceState.depthFailOp = WGPUStencilOperation_Keep;
    stencilFaceState.passOp = WGPUStencilOperation_Keep;

}
void setDefault(WGPUDepthStencilState &depthStencilState)
{
    depthStencilState.format = WGPUTextureFormat_Undefined;
    depthStencilState.depthWriteEnabled = false;
    depthStencilState.depthCompare = WGPUCompareFunction_Always;
    depthStencilState.stencilReadMask = 0xFFFFFFFF;
    depthStencilState.stencilWriteMask = 0xFFFFFFFF;
    depthStencilState.depthBias = 0;
    depthStencilState.depthBiasSlopeScale = 0;
    depthStencilState.depthBiasClamp = 0;
    setDefault(depthStencilState.stencilFront);
    setDefault(depthStencilState.stencilBack);
}

void RenderPass::createPipeline()
{
    std::string shaderSource = m_gpu.compileShader("./shaders/colorpass.wgsl");

    // Load the shader module
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
    pipelineDesc.label = "color pass pipeline";
    pipelineDesc.nextInChain = nullptr;

    setDefault(m_depthStencilState);
    m_depthStencilState.depthCompare          = WGPUCompareFunction_Less;
    m_depthStencilState.depthWriteEnabled     = true;
    m_depthStencilState.format                = WGPUTextureFormat_Depth24Plus;
    m_depthStencilState.stencilReadMask       = 0;
    m_depthStencilState.stencilWriteMask      = 0;

    pipelineDesc.depthStencil = &m_depthStencilState;

    WGPUVertexBufferLayout layout = VertexBuffer::Layout().layout;
    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.vertex.buffers = &layout;
    
    pipelineDesc.vertex.module = shaderModule;
    pipelineDesc.vertex.entryPoint = "vs_main";
    pipelineDesc.vertex.constantCount = 0;
    pipelineDesc.vertex.constants = nullptr;

    pipelineDesc.primitive.topology         = WGPUPrimitiveTopology_TriangleList;
    pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;

    pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;

    // But the face orientation does not matter much because we do not
    // cull (i.e. "hide") the faces pointing away from us (which is often
    // used for optimization).
    pipelineDesc.primitive.cullMode = WGPUCullMode_None;

    WGPUFragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;

    WGPUBlendState blendState{};
    blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blendState.color.operation = WGPUBlendOperation_Add;
    blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
    blendState.alpha.dstFactor = WGPUBlendFactor_One;
    blendState.alpha.operation = WGPUBlendOperation_Add;

    WGPUColorTargetState colorTarget{};
    colorTarget.format      = m_renderTarget.m_surfaceFormat;
    colorTarget.blend       = &blendState;
    colorTarget.writeMask   = WGPUColorWriteMask_All;

    fragmentState.targetCount   = 1;
    fragmentState.targets       = &colorTarget;
    pipelineDesc.fragment       = &fragmentState;

    createLayout(pipelineDesc);
    createBindings();

    // Samples per pixel
    pipelineDesc.multisample.count  = 1;
    pipelineDesc.multisample.mask   = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;

    m_pipeline = wgpuDeviceCreateRenderPipeline(m_gpu.m_device, &pipelineDesc);

    wgpuShaderModuleRelease(shaderModule);
}

void RenderPass::createLayout(WGPURenderPipelineDescriptor& pipeline)
{
    WGPUBindGroupLayoutEntry entry0{};
    setDefault(entry0);
    entry0.binding = 0;
    entry0.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    entry0.buffer.type = WGPUBufferBindingType_Uniform;
    entry0.buffer.minBindingSize = sizeof(FrameData);

    WGPUBindGroupLayoutEntry entry1{};
    setDefault(entry1);
    entry1.binding = 0;
    entry1.visibility = WGPUShaderStage_Vertex;
    entry1.buffer.type = WGPUBufferBindingType_Uniform;
    entry1.buffer.minBindingSize = sizeof(ModelData);

    WGPUBindGroupLayoutDescriptor groupLayout0{};
    groupLayout0.nextInChain = nullptr;
    groupLayout0.entryCount = 1;
    groupLayout0.entries = &entry0;
    m_bindGroupLayouts[0] = wgpuDeviceCreateBindGroupLayout(m_gpu.m_device, &groupLayout0);

    WGPUBindGroupLayoutDescriptor groupLayout1{};
    groupLayout1.nextInChain = nullptr;
    groupLayout1.entryCount = 1;
    groupLayout1.entries = &entry1;
    m_bindGroupLayouts[1] = wgpuDeviceCreateBindGroupLayout(m_gpu.m_device, &groupLayout1);

    WGPUPipelineLayoutDescriptor layoutDesc{};
    layoutDesc.nextInChain          = nullptr;
    layoutDesc.bindGroupLayoutCount = 2;
    layoutDesc.bindGroupLayouts     = m_bindGroupLayouts.data();
    m_layout = wgpuDeviceCreatePipelineLayout(m_gpu.m_device, &layoutDesc);

    pipeline.layout = m_layout;
}

void RenderPass::createBindings()
{
    WGPUBindGroupEntry entry0{};
    entry0.nextInChain = nullptr;
    entry0.binding = 0; 
    entry0.buffer = m_uniformsFrame.buffer;
    entry0.offset = 0;
    entry0.size = sizeof(FrameData);

    WGPUBindGroupEntry entry1{};
    entry1.nextInChain = nullptr;
    entry1.binding = 0; 
    entry1.buffer = m_uniformsModel.buffer;
    entry1.offset = 0;
    entry1.size = sizeof(ModelData);

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

bool FrameData::operator==(const FrameData& other) const
{
    return 
        view == other.view &&
        projection == other.projection &&
        viewPositionWorld == other.viewPositionWorld &&
        lightPositionWorld == other.lightPositionWorld;
}

bool FrameData::operator!=(const FrameData& other) const
{
    return !(*this == other);
}

bool ModelData::operator==(const ModelData& other) const
{
    return 
        model == other.model &&
        modelInverse == other.modelInverse;
}

bool ModelData::operator!=(const ModelData& other) const
{
    return !(*this == other);
}

