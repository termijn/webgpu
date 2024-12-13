#include "renderpass.h"
#include <iostream>

RenderPass::RenderPass(Gpu &gpu_, RenderTarget &renderTarget_)
    : gpu           (gpu_)
    , renderTarget  (renderTarget_)
    , m_vertexBuffer(gpu_)
{
    createPipeline();
}

RenderPass::~RenderPass()
{
    wgpuRenderPipelineRelease(pipeline);
}

void RenderPass::render()
{
    WGPUCommandEncoder encoder = gpu.createCommandEncoder();

    WGPUTextureView targetView = renderTarget.getNextTextureView();

    // The attachment part of the render pass descriptor describes the target texture of the pass
    WGPURenderPassColorAttachment renderPassColorAttachment{};
    renderPassColorAttachment.view          = targetView;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp        = WGPULoadOp_Clear;
    renderPassColorAttachment.storeOp       = WGPUStoreOp_Store;
    renderPassColorAttachment.clearValue = WGPUColor{ 0.05, 0.05, 0.05, 1.0 };

#ifndef WEBGPU_BACKEND_WGPU
    renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif

    WGPURenderPassDescriptor renderPassDesc{};
    renderPassDesc.nextInChain              = nullptr;
    renderPassDesc.label                    = "render pass";
    renderPassDesc.colorAttachmentCount     = 1;
    renderPassDesc.colorAttachments         = &renderPassColorAttachment;
    renderPassDesc.depthStencilAttachment   = nullptr;

    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

    wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);

    wgpuRenderPassEncoderSetVertexBuffer(renderPass,  0, m_vertexBuffer.m_vertexBuffer, 0, wgpuBufferGetSize(m_vertexBuffer.m_vertexBuffer));

    drawCommands(renderPass);

    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);

    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.nextInChain = nullptr;
    cmdBufferDescriptor.label       = "Command buffer";
    WGPUCommandBuffer command       = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);

    wgpuCommandEncoderRelease(encoder);

    wgpuQueueSubmit(gpu.m_queue, 1, &command);
    wgpuCommandBufferRelease(command);

#if defined(WEBGPU_BACKEND_DAWN)
    wgpuDeviceTick(gpu.m_device);
#elif defined(WEBGPU_BACKEND_WGPU)
    wgpuDevicePoll(device, false, nullptr);
#endif
}

void RenderPass::drawCommands(WGPURenderPassEncoder encoder)
{
    wgpuRenderPassEncoderDraw(encoder, m_vertexBuffer.m_vertexCount, 1, 0, 0);
}

void RenderPass::createPipeline()
{
    std::string shaderSource = gpu.compileShader("./shaders/colorpass.wgsl");

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
    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(gpu.m_device, &shaderDesc);

    WGPURenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.nextInChain = nullptr;

    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.vertex.buffers = &m_vertexBuffer.m_vertexBufferLayout;
    
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
    colorTarget.format      = renderTarget.m_surfaceFormat;
    colorTarget.blend       = &blendState;
    colorTarget.writeMask   = WGPUColorWriteMask_All;

    fragmentState.targetCount   = 1;
    fragmentState.targets       = &colorTarget;
    pipelineDesc.fragment       = &fragmentState;
    pipelineDesc.depthStencil   = nullptr;

    // Samples per pixel
    pipelineDesc.multisample.count  = 1;
    pipelineDesc.multisample.mask   = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    pipelineDesc.layout             = nullptr;

    pipeline = wgpuDeviceCreateRenderPipeline(gpu.m_device, &pipelineDesc);

    wgpuShaderModuleRelease(shaderModule);
}
