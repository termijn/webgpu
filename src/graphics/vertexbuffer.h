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
    const Mesh* m_mesh;

WGPUBuffer  m_vertexBuffer;
    uint32_t    m_vertexCount;

    WGPUBuffer  m_indexBuffer;
    uint32_t    m_indexCount;

    WGPUVertexBufferLayout  m_vertexBufferLayout = {};
    WGPUVertexAttribute     m_positionAttribute = {};
};