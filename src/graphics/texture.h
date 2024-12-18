#pragma once

#include <webgpu/webgpu.h>
#include <glm/glm.hpp>

class Gpu;

class Texture
{
public:

    enum class Format
    {
        Depth,
        RGBA,
        RGB,
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

    WGPUTextureView&         getTextureView();

private:
    Gpu&                    m_gpu;
    Params                  m_params;
    WGPUTextureDescriptor   m_textureDesc;
    WGPUTexture             m_texture       = nullptr;
    WGPUTextureView         m_textureView   = nullptr;

};