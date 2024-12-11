#pragma once

#include "gpu.h"
#include "rendertarget.h"
#include "vertexbuffer.h"

class RenderPass
{
public:
    RenderPass(Gpu& gpu, RenderTarget& renderTarget);
    virtual ~RenderPass();

    void render();

private:
    Gpu&                gpu;
    RenderTarget&       renderTarget;
    WGPURenderPipeline  pipeline;

    VertexBuffer m_vertexBuffer;

    void createPipeline();
    void drawCommands(WGPURenderPassEncoder encoder);
};
