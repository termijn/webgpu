#include "texture.h"
#include "graphics/gpu.h"

#include <webgpu/webgpu.h>

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
        case Format::RGBAsrgb:
            wgpuFormat = WGPUTextureFormat_RGBA8UnormSrgb;
            break;
        case Format::BGRA:
            wgpuFormat = WGPUTextureFormat_BGRA8UnormSrgb;
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
        textureViewDesc.mipLevelCount = m_textureDesc.mipLevelCount;
        textureViewDesc.dimension = WGPUTextureViewDimension_2D;
        textureViewDesc.format = m_textureDesc.format;
        m_textureView = wgpuTextureCreateView(m_texture, &textureViewDesc);
    }
}

uint32_t bit_width(uint32_t m) 
{
    if (m == 0) 
        return 0;
    else 
    { 
        uint32_t w = 0; while (m >>= 1) ++w; return w; 
    }
}

void Texture::writeMipMaps(
        WGPUExtent3D    textureSize,
        int bytesPerPixel,
        const uint8_t* pixelData)
{
    WGPUQueue& queue = m_gpu.m_queue;

    m_textureDesc.mipLevelCount = bit_width(std::max(textureSize.width, textureSize.height));
    setSize(vec2(textureSize.width, textureSize.height));

    WGPUImageCopyTexture destination {};
    destination.texture = m_texture;
    destination.origin = { 0, 0, 0 };
    destination.aspect = WGPUTextureAspect_All;

    WGPUTextureDataLayout source;
    source.offset = 0;

    WGPUExtent3D mipLevelSize = textureSize;
    std::vector<unsigned char> previousLevelPixels;
    WGPUExtent3D previousMipLevelSize;
    for (uint32_t level = 0; level < m_textureDesc.mipLevelCount; ++level) 
    {
        std::vector<unsigned char> pixels(4 * mipLevelSize.width * mipLevelSize.height);
        if (level == 0) 
        {
            // We cannot really avoid this copy since we need this
            // in previousLevelPixels at the next iteration
            memcpy(pixels.data(), pixelData, pixels.size());
        }
        else 
        {
            // Create mip level data
            for (uint32_t i = 0; i < mipLevelSize.width; ++i) {
                for (uint32_t j = 0; j < mipLevelSize.height; ++j) {
                    unsigned char* p = &pixels[4 * (j * mipLevelSize.width + i)];
                    // Get the corresponding 4 pixels from the previous level
                    unsigned char* p00 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 0))];
                    unsigned char* p01 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 1))];
                    unsigned char* p10 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 0))];
                    unsigned char* p11 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 1))];
                    // Average
                    p[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
                    p[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
                    p[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
                    p[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
                }
            }
        }

        // Upload data to the GPU texture
        destination.mipLevel = level;
        source.bytesPerRow = 4 * mipLevelSize.width;
        source.rowsPerImage = mipLevelSize.height;
        wgpuQueueWriteTexture(queue, &destination, pixels.data(), pixels.size(), &source, &mipLevelSize);

        previousLevelPixels = std::move(pixels);
        previousMipLevelSize = mipLevelSize;
        mipLevelSize.width /= 2;
        mipLevelSize.height /= 2;
    }
}

void Texture::setImage(const Image& image)
{
    assert(image.bytesPerPixel == 4 || image.bytesPerPixel == 3);

    if (image.type == Image::Type::R8) 
    {
        writeMipMaps({uint32_t(image.width), uint32_t(image.height), 1}, 1, image.pixels->data());
    }

    if (image.type == Image::Type::RGBA)
    {
        writeMipMaps({uint32_t(image.width), uint32_t(image.height), 1}, 4, image.pixels->data());
    }
}

WGPUTextureView& Texture::getTextureView()
{
    return m_textureView;
}
