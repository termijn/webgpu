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
        Depth24Plus,
        Depth32,
        RGBAsrgb,
        RGBA,
        BGRA,
        R,
        RG
    };

    enum class Usage
    {
        RenderAttachment,
        TextureBinding,
        CopySrcTextureBinding,
        RenderAndBinding
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
    WGPUTextureDescriptor   m_textureDesc   = {};
    WGPUTexture             m_texture       = nullptr;
    WGPUTextureView         m_textureView   = nullptr;

    void writeMipMaps(
        WGPUExtent3D    textureSize,
        int bytesPerPixel,
        const unsigned char* pixelData);

};