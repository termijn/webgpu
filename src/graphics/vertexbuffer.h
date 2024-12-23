#pragma once

#include <cstdint>
#include <webgpu/webgpu.h>
#include "mesh.h"

class Gpu;

class VertexBuffer
{
friend class RenderPass;
friend class ShadowPass;

public:
    VertexBuffer(Gpu& gpu);
    ~VertexBuffer();

    class Layout
    {
    public:
        Layout();
        std::array<WGPUVertexAttribute, 5>  vertexAttribs = {};
        WGPUVertexBufferLayout              layout = {};
    };

    void setMesh(const Mesh* mesh);
    const Mesh& getMesh() const;

private:
    Gpu &           m_gpu;
    const Mesh*     m_mesh;

    WGPUBuffer  m_vertexBuffer = nullptr;
    WGPUBuffer  m_indexBuffer = nullptr;
};