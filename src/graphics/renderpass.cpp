#include "renderpass.h"
#include <iostream>
#include "renderpasshelpers.h"

using namespace glm;

RenderPass::RenderPass(Gpu &gpu, RenderTarget &renderTarget, DepthTarget& shadowTarget)
    : m_gpu           (gpu)
    , m_renderTarget  (renderTarget)
    , m_shadowTarget  (shadowTarget)
    , m_uniformsFrame (gpu)
    , m_uniformsModel (gpu)
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

void RenderPass::drawCommands(WGPURenderPassEncoder renderPass, const std::vector<const Renderable*>& renderables)
{
    m_uniformsFrame.setSize(1);
    m_uniformsFrame.writeChanges(0, m_frameData);

    m_uniformsModel.setSize(renderables.size());

    int index = 0;
    for (const Renderable* renderable : renderables)
    {
        ModelData data
        {
            .model                   = renderable->object->getSpace().toRoot,
            .modelInverseTranspose   = transpose(renderable->object->getSpace().fromRoot)
        };
        m_uniformsModel.writeChanges(index, data);

        Texture* baseColorTexture = nullptr;
        Texture* occlusionTexture= nullptr;
        Texture* normalsTexture= nullptr;
        Texture* emissiveTexture= nullptr;
        Texture* metallicRoughnessTexture= nullptr;

        if (renderable->material.baseColorTexture.has_value())
        {
            const Image& image = renderable->material.baseColorTexture.value();
            baseColorTexture = &m_gpu.getResourcePool().get(&image, true);
        }

        if (renderable->material.occlusion.has_value())
        {
            const Image& image = renderable->material.occlusion.value();
            occlusionTexture = &m_gpu.getResourcePool().get(&image);
        }

        if (renderable->material.normalMap.has_value())
        {
            const Image& image = renderable->material.normalMap.value();
            normalsTexture = &m_gpu.getResourcePool().get(&image);
        }

        if (renderable->material.emissive.has_value())
        {
            const Image& image = renderable->material.emissive.value();
            emissiveTexture = &m_gpu.getResourcePool().get(&image, true);
        }

        if (renderable->material.metallicRoughness.has_value())
        {
            const Image& image = renderable->material.metallicRoughness.value();
            metallicRoughnessTexture = &m_gpu.getResourcePool().get(&image);
        }

        // TODO: only rebind model group
        createBindings(baseColorTexture, occlusionTexture, normalsTexture, emissiveTexture, metallicRoughnessTexture); 

        uint32_t dataOffset = index * m_gpu.uniformStride(sizeof(ModelData));

        VertexBuffer& vertexBuffer = m_gpu.getResourcePool().get(renderable);

        wgpuRenderPassEncoderSetBindGroup(renderPass, 0, m_bindGroups[0], 0, nullptr);
        wgpuRenderPassEncoderSetBindGroup(renderPass, 1, m_bindGroups[1], 1, &dataOffset);

        wgpuRenderPassEncoderSetIndexBuffer (renderPass, vertexBuffer.m_indexBuffer, WGPUIndexFormat_Uint32, 0, wgpuBufferGetSize(vertexBuffer.m_indexBuffer));
        wgpuRenderPassEncoderSetVertexBuffer(renderPass,  0, vertexBuffer.m_vertexBuffer, 0, wgpuBufferGetSize(vertexBuffer.m_vertexBuffer));
        wgpuRenderPassEncoderDrawIndexed    (renderPass, vertexBuffer.m_mesh->indices().size() * 3, 1, 0, 0, 0);
        index++;
    }
}

void RenderPass::renderPre(const RenderParams& params)
{
    m_frameData.view                = params.view;
    m_frameData.projection          = params.projection;
    m_frameData.viewPositionWorld   = inverse(params.view)[3];
    m_frameData.lightPositionWorld  = params.lightPosWorld;
    m_frameData.shadowViewProjection = params.shadowViewProjection;
}

void RenderPass::render(const std::vector<const Renderable*>& renderables)
{
    WGPUCommandEncoder& encoder = m_gpu.m_currentCommandEncoder;

    WGPUTextureView targetView = m_renderTarget.getNextTextureView();

    WGPURenderPassColorAttachment renderPassColorAttachment{};
    renderPassColorAttachment.view          = m_renderTarget.getMsaaTextureView();
    renderPassColorAttachment.resolveTarget = targetView;
    renderPassColorAttachment.loadOp        = WGPULoadOp_Clear;
    renderPassColorAttachment.storeOp       = WGPUStoreOp_Store;
    renderPassColorAttachment.clearValue    = WGPUColor{ 1.0, 1.0, 1.0, 0.0 };

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

    drawCommands(renderPass, renderables);

    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);
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

    pipelineDesc.multisample.count  = 4;
    pipelineDesc.multisample.mask   = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;

    m_pipeline = wgpuDeviceCreateRenderPipeline(m_gpu.m_device, &pipelineDesc);

    wgpuShaderModuleRelease(shaderModule);
}

void RenderPass::createLayout(WGPURenderPipelineDescriptor& pipeline)
{
    std::array<WGPUBindGroupLayoutEntry, 3> frameEntries{};
    fillUniformsBindGroupLayoutEntry            (frameEntries[0], 0, sizeof(FrameData), false);
    fillDepthTextureBindGroupLayoutEntry        (frameEntries[1], 1);
    fillSamplerComparisonBindGroupLayoutEntry   (frameEntries[2], 2);

    std::array<WGPUBindGroupLayoutEntry, 7> modelEntries{};
    fillUniformsBindGroupLayoutEntry (modelEntries[0], 0, sizeof(ModelData), true);
    fillSamplerBindGroupLayoutEntry  (modelEntries[1], 1);
    fillTextureBindGroupLayoutEntry  (modelEntries[2], 2);
    fillTextureBindGroupLayoutEntry  (modelEntries[3], 3);
    fillTextureBindGroupLayoutEntry  (modelEntries[4], 4);
    fillTextureBindGroupLayoutEntry  (modelEntries[5], 5);
    fillTextureBindGroupLayoutEntry  (modelEntries[6], 6);
    
    WGPUBindGroupLayoutDescriptor groupLayoutFrame{};
    groupLayoutFrame.label = "grouplayout0 - per frame data";
    groupLayoutFrame.nextInChain = nullptr;
    groupLayoutFrame.entryCount = frameEntries.size();
    groupLayoutFrame.entries = frameEntries.data();
    m_bindGroupLayouts[0] = wgpuDeviceCreateBindGroupLayout(m_gpu.m_device, &groupLayoutFrame);

    WGPUBindGroupLayoutDescriptor groupLayoutModel{};
    groupLayoutModel.label = "grouplayout1 - per model data";
    groupLayoutModel.nextInChain = nullptr;
    groupLayoutModel.entryCount = modelEntries.size();
    groupLayoutModel.entries = modelEntries.data();
    m_bindGroupLayouts[1] = wgpuDeviceCreateBindGroupLayout(m_gpu.m_device, &groupLayoutModel);

    WGPUPipelineLayoutDescriptor layoutDesc{};
    layoutDesc.nextInChain          = nullptr;
    layoutDesc.bindGroupLayoutCount = m_bindGroupLayouts.size();
    layoutDesc.bindGroupLayouts     = m_bindGroupLayouts.data();
    m_layout = wgpuDeviceCreatePipelineLayout(m_gpu.m_device, &layoutDesc);

    pipeline.layout = m_layout;
}

void RenderPass::createBindings(Texture* baseColorTexture, Texture* occlusionTexture, Texture* normalsTexture, Texture* emissiveTexture, Texture* metallicRoughnessTexture)
{
    for (WGPUBindGroup& group : m_bindGroups) if (group != nullptr)
        wgpuBindGroupRelease(group);

    std::array<WGPUBindGroupEntry, 3> frameBindings{};
    WGPUBindGroupEntry& entry0 = frameBindings[0];
    entry0.nextInChain = nullptr;
    entry0.binding = 0; 
    entry0.buffer = m_uniformsFrame.m_buffer;
    entry0.offset = 0;
    entry0.size = sizeof(FrameData);

    WGPUBindGroupEntry& entryShadowTexture = frameBindings[1];
    entryShadowTexture.binding = 1;
    entryShadowTexture.textureView = m_shadowTarget.getDepthTextureView();

    WGPUBindGroupEntry& entryDepthSampler = frameBindings[2];
    entryDepthSampler.binding = 2;
    entryDepthSampler.sampler = m_gpu.m_depthSampler;

    std::array<WGPUBindGroupEntry, 7> bindings{};
    
    WGPUBindGroupEntry& entry1 = bindings[0];
    entry1.nextInChain = nullptr;
    entry1.binding = 0; 
    entry1.buffer = m_uniformsModel.m_buffer;
    entry1.offset = 0;
    entry1.size = sizeof(ModelData);

    WGPUBindGroupEntry& entrySampler = bindings[1];
    entrySampler.binding = 1;
    entrySampler.sampler = m_gpu.m_linearSampler;

    if (baseColorTexture != nullptr)
    {
        WGPUBindGroupEntry& entryBaseColorTexture = bindings[2];
        entryBaseColorTexture.binding = 2;
        entryBaseColorTexture.textureView = baseColorTexture->getTextureView();
    }

    if (occlusionTexture != nullptr)
    {
        WGPUBindGroupEntry& entryOcclusionTexture = bindings[3];
        entryOcclusionTexture.binding = 3;
        entryOcclusionTexture.textureView = occlusionTexture->getTextureView();
    }

    if (normalsTexture != nullptr)
    {
        WGPUBindGroupEntry& entryNormalsTexture = bindings[4];
        entryNormalsTexture.binding = 4;
        entryNormalsTexture.textureView = normalsTexture->getTextureView();
    }

    if (emissiveTexture != nullptr)
    {
        WGPUBindGroupEntry& entryEmissiveTexture = bindings[5];
        entryEmissiveTexture.binding = 5;
        entryEmissiveTexture.textureView = emissiveTexture->getTextureView();
    }

    if (metallicRoughnessTexture != nullptr)
    {
        WGPUBindGroupEntry& entryTexture = bindings[6];
        entryTexture.binding = 6;
        entryTexture.textureView = metallicRoughnessTexture->getTextureView();
    }

    WGPUBindGroupDescriptor group0{};
    group0.nextInChain   = nullptr;
    group0.layout        = m_bindGroupLayouts[0];
    group0.entryCount    = frameBindings.size();
    group0.entries       = frameBindings.data();

    WGPUBindGroupDescriptor group1{};
    group1.nextInChain   = nullptr;
    group1.layout        = m_bindGroupLayouts[1];
    group1.entryCount    = bindings.size();
    group1.entries       = bindings.data();

    m_bindGroups[0] = wgpuDeviceCreateBindGroup(m_gpu.m_device, &group0);
    m_bindGroups[1] = wgpuDeviceCreateBindGroup(m_gpu.m_device, &group1);
}
