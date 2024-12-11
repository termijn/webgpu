#pragma once

#include <webgpu/webgpu.h>
#include <string>

class Gpu
{
friend class WindowTarget;
friend class RenderPass;
friend class VertexBuffer;

public:
    Gpu();
    ~Gpu();

    std::string compileShader(const std::string& file);

private:
    WGPUInstance    m_instance;
    WGPUDevice      m_device;
    WGPUAdapter     m_adapter;
    WGPUQueue       m_queue;

    WGPUCommandEncoder createCommandEncoder();

    WGPUDevice  createDevice    (WGPUAdapter adapter);
    WGPUQueue   createQueue     (WGPUDevice device);
    
    void        enumerateDeviceFeatures     (WGPUDevice device);
    void        enumerateAdapterProperties  (WGPUAdapter adapter);
    void        enumerateAdapterFeatures    (WGPUAdapter adapter);
    void        enumerateAdapterLimits      (WGPUAdapter adapter);

    WGPUDevice      requestDeviceSync   (WGPUAdapter adapter, WGPUDeviceDescriptor const *descriptor);
    WGPUAdapter     requestAdapterSync  (WGPUInstance instance, WGPURequestAdapterOptions const *options);
    WGPUAdapter     createAdapter();
    WGPUInstance    createInstance();

    WGPURequiredLimits getRequiredLimits(WGPUAdapter adapter) const;

    WGPUInstance getInstance();

};
