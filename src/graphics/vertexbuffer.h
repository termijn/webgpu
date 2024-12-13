#pragma once

#include <cstdint>
#include <webgpu/webgpu.h>

#include "gpu.h"
#include "mesh.h"

class VertexBuffer
{
friend class RenderPass;

public:
    VertexBuffer(Gpu& gpu);
    ~VertexBuffer();

    void setMesh(const Mesh* mesh);
    const Mesh& getMesh() const;

private:
    Gpu &           m_gpu;
    const Mesh*     m_mesh;

    WGPUBuffer  m_vertexBuffer = nullptr;
    uint32_t    m_vertexCount;

    WGPUBuffer  m_indexBuffer = nullptr;
    uint32_t    m_indexCount;

    WGPUVertexBufferLayout              m_vertexBufferLayout = {};
    std::array<WGPUVertexAttribute, 2>  m_vertexAttribs = {};
};