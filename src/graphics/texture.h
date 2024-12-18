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

    Texture(Gpu& gpu, Format format);
    ~Texture();

    void setSize(glm::vec2 size);

    WGPUTextureView&         getTextureView();

private:
    Gpu&                    m_gpu;
    Format                  m_format;
    WGPUTextureDescriptor   m_textureDesc;
    WGPUTexture             m_texture       = nullptr;
    WGPUTextureView         m_textureView   = nullptr;

};