#include "vertexbuffer.h"
#include <vector>

VertexBuffer::VertexBuffer(Gpu &gpu) 
    : m_gpu(gpu)
{
}

VertexBuffer::~VertexBuffer() 
{
    wgpuBufferRelease(m_vertexBuffer);
    wgpuBufferRelease(m_indexBuffer);
    m_vertexBuffer  = nullptr;
    m_indexBuffer   = nullptr;
}

void VertexBuffer::setMesh(const Mesh *mesh) 
{
    if (m_vertexBuffer != nullptr)
    {
        wgpuBufferRelease(m_vertexBuffer);
        m_vertexBuffer = nullptr;
    }

    if (m_indexBuffer != nullptr)
    {
        wgpuBufferRelease(m_indexBuffer);
        m_indexBuffer = nullptr;
    }

    m_mesh = mesh;

    // Write vertex buffer
    WGPUBufferDescriptor bufferDesc{};
    bufferDesc.nextInChain          = nullptr;
    bufferDesc.size                 = mesh->vertices().size() * sizeof(Vertex);
    bufferDesc.usage                = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
    bufferDesc.mappedAtCreation     = false;

    m_vertexBuffer = wgpuDeviceCreateBuffer(m_gpu.m_device, &bufferDesc);
    wgpuQueueWriteBuffer(m_gpu.m_queue, m_vertexBuffer, 0, mesh->vertices().data(), bufferDesc.size);

    // Write index buffer
    WGPUBufferDescriptor indexBufferDesc{};
    indexBufferDesc.nextInChain          = nullptr;
    indexBufferDesc.size                 = mesh->indices().size() * sizeof(glm::u32vec3);
    indexBufferDesc.size                 = (indexBufferDesc.size + 3) & ~3; // round up to the next multiple of 4
    indexBufferDesc.usage                = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
    indexBufferDesc.mappedAtCreation     = false;

    m_indexBuffer = wgpuDeviceCreateBuffer(m_gpu.m_device, &indexBufferDesc);
    wgpuQueueWriteBuffer(m_gpu.m_queue, m_indexBuffer, 0, mesh->indices().data(), indexBufferDesc.size);

    // Position
    m_vertexAttribs[0].shaderLocation = 0;
    m_vertexAttribs[0].format = WGPUVertexFormat::WGPUVertexFormat_Float32x4;
    m_vertexAttribs[0].offset = offsetof(Vertex, position);

    // Normal
    m_vertexAttribs[1].shaderLocation = 1;
    m_vertexAttribs[1].format = WGPUVertexFormat::WGPUVertexFormat_Float32x4;
    m_vertexAttribs[1].offset = offsetof(Vertex, normal);

    m_vertexBufferLayout.attributeCount = m_vertexAttribs.size();
    m_vertexBufferLayout.attributes     = m_vertexAttribs.data();
    m_vertexBufferLayout.arrayStride    = sizeof(Vertex);
    m_vertexBufferLayout.stepMode       = WGPUVertexStepMode_Vertex;
}

const Mesh &VertexBuffer::getMesh() const { return *m_mesh; }
