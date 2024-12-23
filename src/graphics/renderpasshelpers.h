#pragma once

#include <webgpu/webgpu.h>

void fillTextureBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& entry, int binding);
void fillDepthTextureBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& entry, int binding);

void fillSamplerBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& entry, int binding);
void fillSamplerComparisonBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& entry, int binding);

void fillUniformsBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& entry, int binding, int size, bool dynamicOffset);

void setDefault(WGPUBindGroupLayoutEntry &bindingLayout);
void setDefault(WGPUStencilFaceState &stencilFaceState);
void setDefault(WGPUDepthStencilState &depthStencilState);