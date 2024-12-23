#pragma once

#include <webgpu/webgpu.h>
#include "gpu.h"

template <typename T>
class Uniforms
{
friend class RenderPass;
friend class ShadowPass;

public:
    Uniforms(Gpu& gpu)
        : m_gpu(gpu)
    {
    }

    virtual ~Uniforms()
    {
        if (m_buffer != nullptr)
            wgpuBufferRelease(m_buffer);
        m_buffer = nullptr;
    }

    bool setSize(int newSize)
    {
        if (newSize == m_lastWrittenData.size() && m_buffer != nullptr)
            return false;

        if (m_buffer != nullptr)
            wgpuBufferRelease(m_buffer);

        uint32_t stride = m_gpu.uniformStride(sizeof(T));
        m_bufferDesc.size               = stride * newSize;
        m_bufferDesc.usage              = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
        m_bufferDesc.mappedAtCreation   = false;
        m_buffer = wgpuDeviceCreateBuffer(m_gpu.m_device, &m_bufferDesc);

        m_lastWrittenData   .resize(newSize);
        m_initialized       .resize(newSize, false);
        return true;
    }

    void writeChanges(int index, const T& data)
    {
        assert(index < m_lastWrittenData.size());
        uint32_t stride = m_gpu.uniformStride(sizeof(T));
        uint32_t offset = stride * index;
        if (m_lastWrittenData[index] != data || !m_initialized[index])
            wgpuQueueWriteBuffer(m_gpu.m_queue, m_buffer, offset, &data, sizeof(T));

        m_initialized[index]     = true;
        m_lastWrittenData[index] = data;
    }
private:
    Gpu&                 m_gpu;
    WGPUBufferDescriptor m_bufferDesc{};
    std::vector<T>       m_lastWrittenData{};
    std::vector<bool>    m_initialized {};

    WGPUBuffer m_buffer = nullptr;
};