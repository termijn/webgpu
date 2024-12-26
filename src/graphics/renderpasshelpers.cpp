#include "renderpasshelpers.h"

void fillTextureBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& entry, int binding)
{
    setDefault(entry);
    entry.binding               = binding;
    entry.visibility            = WGPUShaderStage_Fragment;
    entry.texture.sampleType    = WGPUTextureSampleType_Float;
    entry.texture.viewDimension = WGPUTextureViewDimension_2D;
    entry.texture.multisampled  = false;
}

void fillTextureCubeBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& entry, int binding)
{
    setDefault(entry);
    entry.binding               = binding;
    entry.visibility            = WGPUShaderStage_Fragment;
    entry.texture.sampleType    = WGPUTextureSampleType_Float;
    entry.texture.viewDimension = WGPUTextureViewDimension_Cube;
    entry.texture.multisampled  = false;
}

void fillDepthTextureBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& entry, int binding)
{
    setDefault(entry);
    entry.binding               = binding;
    entry.visibility            = WGPUShaderStage_Fragment;
    entry.texture.sampleType    = WGPUTextureSampleType_Depth;
    entry.texture.viewDimension = WGPUTextureViewDimension_2D;
    entry.texture.multisampled  = false;
}

void fillSamplerBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& entry, int binding)
{
    setDefault(entry);
    entry.binding = binding;
    entry.visibility = WGPUShaderStage_Fragment;
    entry.sampler.type = WGPUSamplerBindingType_Filtering;
}

void fillSamplerComparisonBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& entry, int binding)
{
    setDefault(entry);
    entry.binding = binding;
    entry.visibility = WGPUShaderStage_Fragment;
    entry.sampler.type = WGPUSamplerBindingType_Comparison;
}

void fillUniformsBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& entry, int binding, int size, bool dynamicOffset)
{
    setDefault(entry);
    entry.binding = 0;
    entry.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    entry.buffer.type = WGPUBufferBindingType_Uniform;
    entry.buffer.minBindingSize = size;
    entry.buffer.hasDynamicOffset = dynamicOffset;
}

void setDefault(WGPUBindGroupLayoutEntry &bindingLayout) 
{
    bindingLayout.buffer.nextInChain = nullptr;
    bindingLayout.buffer.type = WGPUBufferBindingType_Undefined;
    bindingLayout.buffer.hasDynamicOffset = false;

    bindingLayout.sampler.nextInChain = nullptr;
    bindingLayout.sampler.type = WGPUSamplerBindingType_Undefined;

    bindingLayout.storageTexture.nextInChain = nullptr;
    bindingLayout.storageTexture.access = WGPUStorageTextureAccess_Undefined;
    bindingLayout.storageTexture.format = WGPUTextureFormat_Undefined;
    bindingLayout.storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

    bindingLayout.texture.nextInChain = nullptr;
    bindingLayout.texture.multisampled = false;
    bindingLayout.texture.sampleType = WGPUTextureSampleType_Undefined;
    bindingLayout.texture.viewDimension = WGPUTextureViewDimension_Undefined;
}

void setDefault(WGPUStencilFaceState &stencilFaceState)
{
    stencilFaceState.compare = WGPUCompareFunction_Always;
    stencilFaceState.failOp = WGPUStencilOperation_Keep;
    stencilFaceState.depthFailOp = WGPUStencilOperation_Keep;
    stencilFaceState.passOp = WGPUStencilOperation_Keep;

}

void setDefault(WGPUDepthStencilState &depthStencilState)
{
    depthStencilState.format = WGPUTextureFormat_Undefined;
    depthStencilState.depthWriteEnabled = false;
    depthStencilState.depthCompare = WGPUCompareFunction_Always;
    depthStencilState.stencilReadMask = 0xFFFFFFFF;
    depthStencilState.stencilWriteMask = 0xFFFFFFFF;
    depthStencilState.depthBias = 0;
    depthStencilState.depthBiasSlopeScale = 0;
    depthStencilState.depthBiasClamp = 0;
    setDefault(depthStencilState.stencilFront);
    setDefault(depthStencilState.stencilBack);
}