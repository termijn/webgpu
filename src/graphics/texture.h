#pragma once

#include <webgpu/webgpu.h>
#include <glm/glm.hpp>
#include "image.h"

class Gpu;

class Texture
{
public:

    enum class Format
    {
        Depth,
        RGBA,
        BGRA,
        R,
        RG
    };

    enum class Usage
    {
        RenderAttachment,
        TextureBinding,
        CopySrcTextureBinding
    };

    struct Params
    {
        Format  format = Format::RGBA;
        Usage   usage  = Usage::TextureBinding;
        int     sampleCount = 1;
    };

    Texture(Gpu& gpu, Params params);
    ~Texture();

    void setSize(glm::vec2 size);
    void setImage(const Image& image);

    WGPUTextureView&         getTextureView();

private:
    Gpu&                    m_gpu;
    Params                  m_params;
    WGPUTextureDescriptor   m_textureDesc;
    WGPUTexture             m_texture       = nullptr;
    WGPUTextureView         m_textureView   = nullptr;

    void writeMipMaps(
        WGPUExtent3D    textureSize,
        [[maybe_unused]] uint32_t mipLevelCount, // not used yet
        int bytesPerPixel,
        const unsigned char* pixelData);

};