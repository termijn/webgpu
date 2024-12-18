#include "vertexbuffer.h"
#include <vector>

#include "graphics/gpu.h"

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
}

const Mesh &VertexBuffer::getMesh() const { return *m_mesh; }

VertexBuffer::Layout::Layout()
{
    // Position
    vertexAttribs[0].shaderLocation = 0;
    vertexAttribs[0].format = WGPUVertexFormat::WGPUVertexFormat_Float32x4;
    vertexAttribs[0].offset = offsetof(Vertex, position);

    // Normal
    vertexAttribs[1].shaderLocation = 1;
    vertexAttribs[1].format = WGPUVertexFormat::WGPUVertexFormat_Float32x4;
    vertexAttribs[1].offset = offsetof(Vertex, normal);

    vertexAttribs[2].shaderLocation = 2;
    vertexAttribs[2].format = WGPUVertexFormat::WGPUVertexFormat_Float32x2;
    vertexAttribs[2].offset = offsetof(Vertex, uv);

    vertexAttribs[3].shaderLocation = 3;
    vertexAttribs[3].format = WGPUVertexFormat::WGPUVertexFormat_Float32x4;
    vertexAttribs[3].offset = offsetof(Vertex, tangent);

    vertexAttribs[4].shaderLocation = 4;
    vertexAttribs[4].format = WGPUVertexFormat::WGPUVertexFormat_Float32x4;
    vertexAttribs[4].offset = offsetof(Vertex, bitangent);

    layout.attributeCount = vertexAttribs.size();
    layout.attributes     = vertexAttribs.data();
    layout.arrayStride    = sizeof(Vertex);
    layout.stepMode       = WGPUVertexStepMode_Vertex;
}