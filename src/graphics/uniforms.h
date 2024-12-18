#pragma once

#include <webgpu/webgpu.h>
#include "gpu.h"

// TODO: rename to bind group and add bind group layout and creation
template <typename T>
class Uniforms
{
friend class RenderPass;
public:
    Uniforms(Gpu& gpu, T& data)
        : m_gpu(gpu)
        , m_data(data)
    {
        WGPUBufferDescriptor bufferDesc{};
        bufferDesc.size = sizeof(T);
        bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
        bufferDesc.mappedAtCreation = false;
        buffer = wgpuDeviceCreateBuffer(gpu.m_device, &bufferDesc);
    }

    virtual ~Uniforms()
    {
        wgpuBufferRelease(buffer);
        buffer = nullptr;
    }

    void upload()
    {
        if (m_lastWrittenData != m_data || !m_dataInitialized)
            wgpuQueueWriteBuffer(m_gpu.m_queue, buffer, 0, &m_data, sizeof(T));

        m_dataInitialized = true;
        m_lastWrittenData = m_data;
    }
private:
    Gpu&    m_gpu;
    T&      m_data;
    T       m_lastWrittenData;
    bool    m_dataInitialized = false;

    WGPUBuffer buffer = nullptr;
};