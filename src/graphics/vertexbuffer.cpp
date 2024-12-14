#include "vertexbuffer.h"
#include <vector>

VertexBuffer::VertexBuffer(Gpu &gpu) 
    : m_gpu(gpu)
{
    std::vector<float> vertexData = {
        -0.5, 0.5, 1.0, 0.0, 0.0,
        -0.5, -0.5, 0.0, 1.0, 0.0,
         0.5,  0.5, 0.0, 0.0, 1.0,
         0.5, -0.5, 0.0, 1.0, 0.0,
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

    // Define index data
    std::vector<uint32_t> indexData = {
        0, 1, 2,
        1, 3, 2 
    };

    m_indexCount = indexData.size();

    WGPUBufferDescriptor indexBufferDesc{};
    indexBufferDesc.nextInChain          = nullptr;
    indexBufferDesc.size                 = m_indexCount * sizeof(uint32_t);
    indexBufferDesc.size                 = (indexBufferDesc.size + 3) & ~3; // round up to the next multiple of 4
    indexBufferDesc.usage                = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index; // Vertex usage here!
    indexBufferDesc.mappedAtCreation     = false;
    

    m_indexBuffer = wgpuDeviceCreateBuffer(gpu.m_device, &indexBufferDesc);
    wgpuQueueWriteBuffer(gpu.m_queue, m_indexBuffer, 0, indexData.data(), indexBufferDesc.size);
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
