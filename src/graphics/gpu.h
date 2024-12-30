#pragma once

#include <webgpu/webgpu.h>
#include <string>

#include "resourcepool.h"

class Gpu
{
friend class WindowTarget;
friend class RenderPass;
friend class ShadowPass;
friend class VertexBuffer;
friend class Texture;
template <typename T>
friend class Uniforms;

public:
    Gpu();
    ~Gpu();

    ResourcePool& getResourcePool();

    std::string compileShader(const std::string& file);

    uint32_t uniformStride(uint32_t uniformSize) const;

    void beginRenderJob();
    void submitRenderJob();

private:
    WGPUInstance        m_instance;
    WGPUDevice          m_device;
    WGPUAdapter         m_adapter;
    WGPUQueue           m_queue;
    WGPURequiredLimits  m_requiredLimits{};
    
    WGPUSampler         m_linearSampler;
    WGPUSampler         m_nearestSampler;
    WGPUSampler         m_depthSampler;

    WGPUCommandEncoder m_currentCommandEncoder = nullptr;

    std::unique_ptr<ResourcePool> m_resourcePool;

    WGPUCommandEncoder createCommandEncoder();

    WGPUDevice  createDevice    (WGPUAdapter adapter);
    WGPUQueue   createQueue     (WGPUDevice device);
    
    WGPUSampler createLinearSampler(WGPUDevice device);
    WGPUSampler createDepthSampler(WGPUDevice device);
    WGPUSampler createNearestSampler(WGPUDevice device);
    
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
