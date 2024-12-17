#include "texture.h"

Texture::Texture(Gpu& gpu, Format format)
    : m_gpu     (gpu)
    , m_format  (format)
{
    WGPUTextureFormat wgpuFormat;
    WGPUTextureUsageFlags wgpuUsage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding;
    switch (format)
    {
        case Format::Depth:
            wgpuFormat = WGPUTextureFormat_Depth24Plus;
            wgpuUsage = WGPUTextureUsage_RenderAttachment;
            break;
        case Format::RGBA:
            wgpuFormat = WGPUTextureFormat_RGBA32Uint;
            break;
        case Format::RGB:
            wgpuFormat = WGPUTextureFormat_ETC2RGB8Unorm;
            break;
    }
    m_textureDesc.nextInChain = nullptr;
    m_textureDesc.dimension = WGPUTextureDimension_2D;
    m_textureDesc.format = wgpuFormat;
    m_textureDesc.mipLevelCount = 1;
    m_textureDesc.sampleCount = 1;
    m_textureDesc.size = {1, 1, 1};
    m_textureDesc.usage = wgpuUsage;
    m_textureDesc.viewFormatCount = 1;
    m_textureDesc.viewFormats = &m_textureDesc.format;
}

Texture::~Texture() 
{
    if (m_texture != nullptr)
    {
        wgpuTextureViewRelease(m_textureView);
        wgpuTextureDestroy(m_texture);
        wgpuTextureRelease(m_texture);
    }
    m_texture       = nullptr;
    m_textureView   = nullptr;
};

void Texture::setSize(glm::vec2 size)
{
    glm::uvec2 newSize (static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));

    if (newSize.x != m_textureDesc.size.width || newSize.y != m_textureDesc.size.height)
    {
        if (m_texture != nullptr)
        {
            wgpuTextureViewRelease(m_textureView);
            wgpuTextureDestroy(m_texture);
            wgpuTextureRelease(m_texture);
        }

        m_textureDesc.size = WGPUExtent3D { .width = newSize.x, .height = newSize.y, .depthOrArrayLayers = 1 };
        m_texture = wgpuDeviceCreateTexture(m_gpu.m_device, &m_textureDesc);

        WGPUTextureViewDescriptor textureViewDesc;
        textureViewDesc.nextInChain = nullptr;
        textureViewDesc.aspect = WGPUTextureAspect_DepthOnly;
        textureViewDesc.baseArrayLayer = 0;
        textureViewDesc.arrayLayerCount = 1;
        textureViewDesc.baseMipLevel = 0;
        textureViewDesc.mipLevelCount = 1;
        textureViewDesc.dimension = WGPUTextureViewDimension_2D;
        textureViewDesc.format = m_textureDesc.format;
        m_textureView = wgpuTextureCreateView(m_texture, &textureViewDesc);
    }

}

WGPUTextureView& Texture::getTextureView()
{
    return m_textureView;
}
