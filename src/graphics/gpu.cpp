#include "gpu.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <fstream>

#include "wgpu-utils.h"
#include "mesh.h"

Gpu::Gpu()
    : m_resourcePool(std::make_unique<ResourcePool>(*this))
{
    m_instance = createInstance();
    std::cout << "WGPU instance: " << m_instance << std::endl;
    m_adapter = createAdapter();
    std::cout << "Found adapter: " << m_adapter << std::endl;
    wgpuInstanceRelease(m_instance);

    enumerateAdapterLimits(m_adapter);
    enumerateAdapterFeatures(m_adapter);
    enumerateAdapterProperties(m_adapter);

    m_device = createDevice(m_adapter);

    enumerateDeviceFeatures(m_device);

    m_queue = createQueue(m_device);

    m_linearSampler = createLinearSampler(m_device);
}

Gpu::~Gpu()
{
    m_resourcePool = nullptr;
    wgpuSamplerRelease(m_linearSampler);
    wgpuQueueRelease(m_queue);
    wgpuAdapterRelease(m_adapter);
    wgpuDeviceRelease(m_device);
}

ResourcePool& Gpu::getResourcePool()
{
    return *m_resourcePool;
}

std::string Gpu::compileShader(const std::string& filepath)
{
    std::string result;
    std::ifstream stream(filepath);
    if (!stream.is_open())
    {
        std::cerr << "Error: Could not open " << filepath << std::endl;
        return result;
    }
    std::string line;

    while (std::getline(stream, line))
    {
        result += line + "\n";
    }
    stream.close();
    return result;
}

uint32_t ceilToNextMultiple(uint32_t value, uint32_t step) {
    uint32_t divide_and_ceil = value / step + (value % step == 0 ? 0 : 1);
    return step * divide_and_ceil;
}

uint32_t Gpu::uniformStride(uint32_t uniformSize) const
{
    uint32_t uniformStride = ceilToNextMultiple(
      uniformSize,
      m_requiredLimits.limits.minUniformBufferOffsetAlignment);
    return uniformStride;
}

WGPUInstance Gpu::getInstance()
{
    return m_instance;
}

WGPUQueue Gpu::createQueue(WGPUDevice device)
{
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    return queue;
}

WGPUSampler Gpu::createLinearSampler(WGPUDevice device)
{
    WGPUSamplerDescriptor samplerDesc{};
    samplerDesc.nextInChain = nullptr;
    samplerDesc.label = "linearSampler";
    samplerDesc.addressModeU = WGPUAddressMode_Repeat;
    samplerDesc.addressModeV = WGPUAddressMode_Repeat;
    samplerDesc.addressModeW = WGPUAddressMode_Repeat;
    samplerDesc.magFilter = WGPUFilterMode_Linear;
    samplerDesc.minFilter = WGPUFilterMode_Linear;
    samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
    samplerDesc.lodMinClamp = 0.0f;
    samplerDesc.lodMaxClamp = 1.0f;
    samplerDesc.compare = WGPUCompareFunction_Undefined;
    samplerDesc.maxAnisotropy = 4;
    WGPUSampler linearSampler = wgpuDeviceCreateSampler(m_device, &samplerDesc);
    return linearSampler;
}

void Gpu::enumerateDeviceFeatures(WGPUDevice device)
{
    size_t featureCount = wgpuDeviceEnumerateFeatures(device, nullptr);
    std::vector<WGPUFeatureName> features(featureCount);
    wgpuDeviceEnumerateFeatures(device, features.data());

    std::cout << "Device features:" << std::endl;
    std::cout << std::hex;
    for (auto f : features)
    {
        std::cout << " - 0x" << f << std::endl;
    }
    std::cout << std::dec;

    WGPUSupportedLimits limits = {};
    limits.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_DAWN
    bool success = wgpuDeviceGetLimits(device, &limits) == WGPUStatus_Success;
#else
    bool success = wgpuDeviceGetLimits(device, &limits);
#endif

    if (success)
    {
        std::cout << "Device limits:" << std::endl;
        std::cout << " - maxTextureDimension1D: " << limits.limits.maxTextureDimension1D << std::endl;
        std::cout << " - maxTextureDimension2D: " << limits.limits.maxTextureDimension2D << std::endl;
        std::cout << " - maxTextureDimension3D: " << limits.limits.maxTextureDimension3D << std::endl;
        std::cout << " - maxTextureArrayLayers: " << limits.limits.maxTextureArrayLayers << std::endl;
        // [...] Extra device limits
    }
}

WGPUCommandEncoder Gpu::createCommandEncoder()
{
    WGPUCommandEncoderDescriptor encoderDesc;
    encoderDesc.nextInChain = nullptr;
    encoderDesc.label = "Command encoder";
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device, &encoderDesc);
    return encoder;
}

WGPUDevice Gpu::createDevice(WGPUAdapter adapter)
{
    m_requiredLimits = getRequiredLimits(adapter);

    WGPUDeviceDescriptor deviceDesc = {};
    deviceDesc.nextInChain = nullptr;
    deviceDesc.label = "Video device";
    deviceDesc.requiredFeatureCount = 0; // we do not require any specific feature
    deviceDesc.requiredLimits = &m_requiredLimits; // we do not require any specific limit
    deviceDesc.defaultQueue.nextInChain = nullptr;
    deviceDesc.defaultQueue.label = "Default queue";
    deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */)
        {
            std::cout << "Device lost: reason " << reason;
            if (message)
                std::cout << " (" << message << ")";
            std::cout << std::endl;
        };

    WGPUDevice result = requestDeviceSync(adapter, &deviceDesc);

    std::cout << "Got device: " << result << std::endl;

    auto onDeviceError = [](WGPUErrorType type, char const* message, void* /* pUserData */)
        {
            std::cout << "Uncaptured device error: type " << type;
            if (message)
                std::cout << " (" << message << ")";
            std::cout << std::endl;
        };
    wgpuDeviceSetUncapturedErrorCallback(result, onDeviceError, nullptr /* pUserData */);

    return result;
}

void Gpu::enumerateAdapterProperties(WGPUAdapter adapter)
{
    WGPUAdapterProperties properties;
    properties.nextInChain = nullptr;

    wgpuAdapterGetProperties(adapter, &properties);
    std::cout << "Video Adapter properties:" << std::endl;
    std::cout << " - vendorID: " << properties.vendorID << std::endl;
    if (properties.vendorName)
    {
        std::cout << " - vendorName: " << properties.vendorName << std::endl;
    }
    if (properties.architecture)
    {
        std::cout << " - architecture: " << properties.architecture << std::endl;
    }
    std::cout << " - deviceID: " << properties.deviceID << std::endl;
    if (properties.name)
    {
        std::cout << " - name: " << properties.name << std::endl;
    }
    if (properties.driverDescription)
    {
        std::cout << " - driverDescription: " << properties.driverDescription << std::endl;
    }
    std::cout << std::hex;
    std::cout << " - adapterType: 0x" << properties.adapterType << std::endl;
    std::cout << " - backendType: 0x" << properties.backendType << std::endl;
    std::cout << std::dec; // Restore decimal numbers
}

void Gpu::enumerateAdapterFeatures(WGPUAdapter adapter)
{
    size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, nullptr);

    std::vector<WGPUFeatureName> features(featureCount);
    wgpuAdapterEnumerateFeatures(adapter, features.data());

    std::cout << "Adapter features:" << std::endl;
    std::cout << std::hex;
    for (auto f : features)
    {
        std::cout << " - 0x" << f << std::endl;
    }
    std::cout << std::dec;
}

void Gpu::enumerateAdapterLimits(WGPUAdapter adapter)
{
#ifndef __EMSCRIPTEN__
    WGPUSupportedLimits supportedLimits = {};
    supportedLimits.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_DAWN
    bool success = wgpuAdapterGetLimits(adapter, &supportedLimits) == WGPUStatus_Success;
#else
    bool success = wgpuAdapterGetLimits(adapter, &supportedLimits);
#endif

    if (success)
    {
        std::cout << "Adapter limits:" << std::endl;
        std::cout << " - maxTextureDimension1D: " << supportedLimits.limits.maxTextureDimension1D << std::endl;
        std::cout << " - maxTextureDimension2D: " << supportedLimits.limits.maxTextureDimension2D << std::endl;
        std::cout << " - maxTextureDimension3D: " << supportedLimits.limits.maxTextureDimension3D << std::endl;
        std::cout << " - maxTextureArrayLayers: " << supportedLimits.limits.maxTextureArrayLayers << std::endl;
    }
    else
    {
        std::cout << "Could not get adapter limits" << std::endl;
    }
#endif
}

WGPUDevice Gpu::requestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const* descriptor)
{
    struct UserData
    {
        WGPUDevice device = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* pUserData)
        {
            UserData& userData = *reinterpret_cast<UserData*>(pUserData);
            if (status == WGPURequestDeviceStatus_Success)
            {
                userData.device = device;
            }
            else
            {
                std::cout << "Could not get WebGPU device: " << message << std::endl;
            }
            userData.requestEnded = true;
        };

    wgpuAdapterRequestDevice(
        adapter,
        descriptor,
        onDeviceRequestEnded,
        (void*)&userData);

#ifdef __EMSCRIPTEN__
    while (!userData.requestEnded)
    {
        emscripten_sleep(100);
    }
#endif // __EMSCRIPTEN__

    assert(userData.requestEnded);

    return userData.device;
}

WGPUAdapter Gpu::requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const* options)
{
    struct UserData
    {
        WGPUAdapter adapter = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* pUserData)
        {
            UserData& userData = *reinterpret_cast<UserData*>(pUserData);
            if (status == WGPURequestAdapterStatus_Success)
                userData.adapter = adapter;
            else
                std::cout << "Could not get WebGPU adapter: " << message << std::endl;
            userData.requestEnded = true;
        };

    wgpuInstanceRequestAdapter(
        instance,
        options,
        onAdapterRequestEnded,
        (void*)&userData);

    // For native dawn, wgpuInstanceRequestAdapter returns after calling the ended callback
    // For emscripten, we need to wait until the request is done.
#ifdef __EMSCRIPTEN__
    while (!userData.requestEnded)
    {
        emscripten_sleep(100);
    }
#endif // __EMSCRIPTEN__

    assert(userData.requestEnded);

    return userData.adapter;
}

WGPUAdapter Gpu::createAdapter()
{
    WGPURequestAdapterOptions adapterOpts = {};
    adapterOpts.nextInChain     = nullptr;
    adapterOpts.powerPreference = WGPUPowerPreference_HighPerformance;
    WGPUAdapter adapter         = requestAdapterSync(m_instance, &adapterOpts);
    return adapter;
}

WGPUInstance Gpu::createInstance()
{
    WGPUInstance instance = nullptr;

#ifdef WEBGPU_BACKEND_EMSCRIPTEN
    instance = wgpuCreateInstance(nullptr);
#else
    WGPUInstanceDescriptor desc = {};
    desc.nextInChain = nullptr;
#ifdef WEBGPU_BACKEND_DAWN

    // Make sure the uncaptured error callback is called as soon as an error
    // occurs rather than at the next call to "wgpuDeviceTick".
    WGPUDawnTogglesDescriptor toggles;
    toggles.chain.next = nullptr;
    toggles.chain.sType = WGPUSType_DawnTogglesDescriptor;
    toggles.disabledToggleCount = 0;
    toggles.enabledToggleCount = 1;
    const char* toggleName = "enable_immediate_error_handling";
    toggles.enabledToggles = &toggleName;

    desc.nextInChain = &toggles.chain;
#endif // WEBGPU_BACKEND_DAWN

    instance = wgpuCreateInstance(&desc);
#endif

    return instance;
}

WGPURequiredLimits Gpu::getRequiredLimits(WGPUAdapter adapter) const
{
    WGPUSupportedLimits supportedLimits;
    supportedLimits.nextInChain = nullptr;
    wgpuAdapterGetLimits(adapter, &supportedLimits);

    WGPURequiredLimits requiredLimits{};
    setDefaults(requiredLimits.limits);

    requiredLimits.limits.maxVertexAttributes           = 5;
    requiredLimits.limits.maxVertexBuffers              = 1;
    requiredLimits.limits.maxBufferSize                 = supportedLimits.limits.maxBufferSize;
    requiredLimits.limits.maxVertexBufferArrayStride    = sizeof(Vertex);
    requiredLimits.limits.maxInterStageShaderComponents = 3;
    
    requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
    requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;

    requiredLimits.limits.maxBindGroups                     = 2;
    requiredLimits.limits.maxUniformBuffersPerShaderStage   = 1;
    requiredLimits.limits.maxUniformBufferBindingSize       = sizeof(float) * 16 * 16;
    requiredLimits.limits.maxDynamicUniformBuffersPerPipelineLayout = 2;

    requiredLimits.limits.maxTextureDimension1D = 2048;
    requiredLimits.limits.maxTextureDimension2D = 2048;
    requiredLimits.limits.maxTextureArrayLayers = 1;
    requiredLimits.limits.maxSamplersPerShaderStage = 1;

    return requiredLimits;
}