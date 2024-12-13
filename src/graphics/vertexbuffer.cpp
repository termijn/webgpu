#include "vertexbuffer.h"
#include <vector>

VertexBuffer::VertexBuffer(Gpu &gpu) 
    : m_gpu(gpu)
{
    // Vertex buffer data
    // There are 2 floats per vertex, one for x and one for y.
    std::vector<float> vertexData = {
        // x0,  y0,  r0,  g0,  b0
        -0.5, -0.5, 1.0, 0.0, 0.0,

        // x1,  y1,  r1,  g1,  b1
        +0.5, -0.5, 0.0, 1.0, 0.0,

        // ...
        +0.0,   +0.5, 0.0, 0.0, 1.0,
        -0.55f, -0.5, 1.0, 1.0, 0.0,
        -0.05f, +0.5, 1.0, 0.0, 1.0,
        -0.55f, +0.5, 0.0, 1.0, 1.0
    };
    m_vertexCount = static_cast<uint32_t>(vertexData.size() / 5);

    WGPUBufferDescriptor bufferDesc{};
    bufferDesc.nextInChain          = nullptr;
    bufferDesc.size                 = vertexData.size() * sizeof(float);
    bufferDesc.usage                = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex; // Vertex usage here!
    bufferDesc.mappedAtCreation     = false;

    m_vertexBuffer = wgpuDeviceCreateBuffer(gpu.m_device, &bufferDesc);

    wgpuQueueWriteBuffer(gpu.m_queue, m_vertexBuffer, 0, vertexData.data(), bufferDesc.size);

    m_vertexAttribs[0].shaderLocation = 0;
    m_vertexAttribs[0].format = WGPUVertexFormat::WGPUVertexFormat_Float32x2;
    m_vertexAttribs[0].offset = 0;

    m_vertexAttribs[1].shaderLocation = 1;
    m_vertexAttribs[1].format = WGPUVertexFormat::WGPUVertexFormat_Float32x3;
    m_vertexAttribs[1].offset = 2 * sizeof(float);

    m_vertexBufferLayout.attributeCount = m_vertexAttribs.size();
    m_vertexBufferLayout.attributes     = m_vertexAttribs.data();
    m_vertexBufferLayout.arrayStride    = 5 * sizeof(float);
    m_vertexBufferLayout.stepMode       = WGPUVertexStepMode_Vertex;
}

VertexBuffer::~VertexBuffer() 
{
    wgpuBufferRelease(m_vertexBuffer);
    m_vertexBuffer = nullptr;
}

void VertexBuffer::setMesh(const Mesh *mesh) 
{
    // if (m_vertexBuffer != nullptr)
    // {
    //     wgpuBufferRelease(m_vertexBuffer);
    //     m_vertexBuffer = nullptr;
    // }

    // m_mesh = mesh;

    // WGPUBufferDescriptor bufferDesc{};
    // bufferDesc.nextInChain          = nullptr;
    // bufferDesc.size                 = m_mesh->vertices().size() * sizeof(Vertex);    
    // bufferDesc.usage                = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex; // Vertex usage here!
    // bufferDesc.mappedAtCreation     = false;

    // m_vertexBuffer = wgpuDeviceCreateBuffer(m_gpu.m_device, &bufferDesc);
}

const Mesh &VertexBuffer::getMesh() const { return *m_mesh; }
