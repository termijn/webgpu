#include "texture.h"
#include "graphics/gpu.h"

using namespace glm;

Texture::Texture(Gpu& gpu, Params params)
    : m_gpu     (gpu)
    , m_params  (params)
{
    WGPUTextureFormat wgpuFormat;
    WGPUTextureUsageFlags wgpuUsage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding;

    switch(params.usage)
    {
        case Usage::RenderAttachment:
            wgpuUsage = WGPUTextureUsage_RenderAttachment;
            break;
        case Usage::CopySrcTextureBinding:
            wgpuUsage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding;
            break;
        case Usage::TextureBinding:
            wgpuUsage = WGPUTextureUsage_TextureBinding;
            break;
        case Usage::RenderAndBinding:
            wgpuUsage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding;
            break;
    }

    switch (params.format)
    {
        case Format::Depth24Plus:
            wgpuFormat = WGPUTextureFormat_Depth24Plus;
            break;
        case Format::Depth32:
            wgpuFormat = WGPUTextureFormat_Depth32Float;
            break;
        case Format::RGBA:
            wgpuFormat = WGPUTextureFormat_RGBA8Unorm;
            break;
        case Format::BGRA:
            wgpuFormat = WGPUTextureFormat_BGRA8Unorm;
            break;
        case Format::R:
            wgpuFormat = WGPUTextureFormat_R8Unorm;
            break;
        case Format::RG:
            wgpuFormat = WGPUTextureFormat_RG8Unorm;
            break;
    }
    m_textureDesc.dimension     = WGPUTextureDimension_2D;
    m_textureDesc.format        = wgpuFormat;
    m_textureDesc.mipLevelCount = 1;
    m_textureDesc.sampleCount   = params.sampleCount;
    m_textureDesc.size          = {1, 1, 1};
    m_textureDesc.usage         = wgpuUsage;

    m_textureDesc.viewFormatCount   = 0;
    m_textureDesc.viewFormats       = nullptr;
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

void Texture::setSize(vec2 size)
{
    uvec2 newSize (static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));

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

        WGPUTextureViewDescriptor textureViewDesc {};
        textureViewDesc.aspect = (m_params.format == Format::Depth24Plus || m_params.format == Format::Depth32) ? WGPUTextureAspect_DepthOnly : WGPUTextureAspect_All;
        textureViewDesc.baseArrayLayer = 0;
        textureViewDesc.arrayLayerCount = 1;
        textureViewDesc.baseMipLevel = 0;
        textureViewDesc.mipLevelCount = 1;
        textureViewDesc.dimension = WGPUTextureViewDimension_2D;
        textureViewDesc.format = m_textureDesc.format;
        m_textureView = wgpuTextureCreateView(m_texture, &textureViewDesc);
    }
}

void Texture::writeMipMaps(
        WGPUExtent3D    textureSize,
        [[maybe_unused]] uint32_t mipLevelCount, // not used yet
        int bytesPerPixel,
        const uint8_t* pixelData)
{
    WGPUImageCopyTexture destination = {};
    destination.texture = m_texture;
    destination.mipLevel = 0;
    destination.origin = { 0, 0, 0 };
    destination.aspect = WGPUTextureAspect_All;

    WGPUTextureDataLayout source = {};
    source.offset = 0; 
    source.bytesPerRow = bytesPerPixel * textureSize.width;
    source.rowsPerImage = textureSize.height;
    source.nextInChain = nullptr;

    wgpuQueueWriteTexture(m_gpu.m_queue, &destination, pixelData, bytesPerPixel * textureSize.width * textureSize.height, &source, &textureSize);
}

void Texture::setImage(const Image& image)
{
    setSize(vec2(image.width, image.height));
    assert(image.bytesPerPixel == 4 || image.bytesPerPixel == 3);

    if (image.type == Image::Type::R8) 
    {
        writeMipMaps({uint32_t(image.width), uint32_t(image.height), 1}, 1, 1, image.pixels->data());
    }

    if (image.type == Image::Type::RGBA)
    {
        writeMipMaps({uint32_t(image.width), uint32_t(image.height), 1}, 1, 4, image.pixels->data());
    }
}

WGPUTextureView& Texture::getTextureView()
{
    return m_textureView;
}
