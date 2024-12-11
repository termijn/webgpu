#include "vertexbuffer.h"
#include <vector>

VertexBuffer::VertexBuffer(Gpu &gpu) 
{
    // Vertex buffer data
    // There are 2 floats per vertex, one for x and one for y.
    std::vector<float> vertexData{
                                -0.5f, -0.5f, 
                                +0.5f, -0.5f, 
                                +0.0f, +0.5f,

                                -0.55f, -0.5f, -0.05f, 
                                +0.5f, -0.55f, +0.5f};
    m_vertexCount = static_cast<uint32_t>(vertexData.size() / 2);

    WGPUBufferDescriptor bufferDesc{};
    bufferDesc.nextInChain          = nullptr;
    bufferDesc.size                 = vertexData.size() * sizeof(float);
    bufferDesc.usage                = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex; // Vertex usage here!
    bufferDesc.mappedAtCreation     = false;

    m_vertexBuffer = wgpuDeviceCreateBuffer(gpu.m_device, &bufferDesc);

    wgpuQueueWriteBuffer(gpu.m_queue, m_vertexBuffer, 0, vertexData.data(), bufferDesc.size);

    m_positionAttribute.shaderLocation    = 0;
    m_positionAttribute.format            = WGPUVertexFormat_Float32x2;
    m_positionAttribute.offset            = 0;

    m_vertexBufferLayout.attributeCount = 1;
    m_vertexBufferLayout.attributes     = &m_positionAttribute;
    m_vertexBufferLayout.arrayStride    = 2 * sizeof(float);
    m_vertexBufferLayout.stepMode       = WGPUVertexStepMode_Vertex;
}

VertexBuffer::~VertexBuffer() 
{
    wgpuBufferRelease(m_vertexBuffer);
}

void VertexBuffer::setMesh(const Mesh *mesh) { m_mesh = mesh; }

const Mesh &VertexBuffer::getMesh() const { return *m_mesh; }
