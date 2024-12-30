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
void Texture::setSize(glm::vec2 size)
{
    setSize(vec3(size, 1));
}

void Texture::setSize(vec3 size)
{
    uvec3 newSize (static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y), static_cast<uint32_t>(size.z));

    if (newSize.x != m_textureDesc.size.width || newSize.y != m_textureDesc.size.height || newSize.z != m_textureDesc.size.depthOrArrayLayers)
    {
        if (m_texture != nullptr)
        {
            wgpuTextureViewRelease(m_textureView);
            wgpuTextureDestroy(m_texture);
            wgpuTextureRelease(m_texture);
        }

        m_textureDesc.size = WGPUExtent3D { .width = newSize.x, .height = newSize.y, .depthOrArrayLayers = newSize.z};
        m_texture = wgpuDeviceCreateTexture(m_gpu.m_device, &m_textureDesc);

        WGPUTextureViewDescriptor textureViewDesc {};
        textureViewDesc.aspect = (m_params.format == Format::Depth24Plus || m_params.format == Format::Depth32) ? WGPUTextureAspect_DepthOnly : WGPUTextureAspect_All;
        textureViewDesc.baseArrayLayer = 0;
        textureViewDesc.arrayLayerCount = newSize.z;
        textureViewDesc.baseMipLevel = 0;
        textureViewDesc.mipLevelCount = m_textureDesc.mipLevelCount;
        textureViewDesc.dimension = size.z == 6 ? WGPUTextureViewDimension_Cube : WGPUTextureViewDimension_2D;
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
    int             bytesPerPixel,
    uint32_t        layer,
    uint32_t        maxMipMaps,
    const uint8_t*  pixelData)
{
    WGPUQueue& queue = m_gpu.m_queue;

    m_textureDesc.mipLevelCount = std::clamp(bit_width(std::max(textureSize.width, textureSize.height)), uint32_t(1), maxMipMaps);
    setSize(vec3(textureSize.width, textureSize.height, textureSize.depthOrArrayLayers));

    WGPUImageCopyTexture destination {};
    destination.texture = m_texture;
    destination.origin = { 0, 0, layer};
    destination.aspect = WGPUTextureAspect_All;

    WGPUExtent3D mipLevelSize = {textureSize.width, textureSize.height, 1};
    std::vector<unsigned char> previousLevelPixels;
    WGPUExtent3D previousMipLevelSize;
    for (uint32_t level = 0; level < m_textureDesc.mipLevelCount; ++level) 
    {
        std::vector<unsigned char> pixels(bytesPerPixel * mipLevelSize.width * mipLevelSize.height);
        if (level == 0) 
        {
            memcpy(pixels.data(), pixelData, pixels.size());
        }
        else 
        {
            for (uint32_t y = 0; y < mipLevelSize.height; y++) 
            {
                for (uint32_t x = 0; x < mipLevelSize.width; x++) 
                {
                    // Get the corresponding 4 pixels from the previous level
                    unsigned char* p00 = &previousLevelPixels[bytesPerPixel * ((2 * y + 0) * previousMipLevelSize.width + (2 * x + 0))];
                    unsigned char* p01 = &previousLevelPixels[bytesPerPixel * ((2 * y + 0) * previousMipLevelSize.width + (2 * x + 1))];
                    unsigned char* p10 = &previousLevelPixels[bytesPerPixel * ((2 * y + 1) * previousMipLevelSize.width + (2 * x + 0))];
                    unsigned char* p11 = &previousLevelPixels[bytesPerPixel * ((2 * y + 1) * previousMipLevelSize.width + (2 * x + 1))];
                    // Average
                    unsigned char* p = &pixels[bytesPerPixel * (y * mipLevelSize.width + x)];
                    p[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
                    p[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
                    p[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
                    p[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
                }
            }
        }

        destination.mipLevel = level;

        WGPUTextureDataLayout source{};
        source.bytesPerRow = bytesPerPixel * mipLevelSize.width;
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
    if (image.type == Image::Type::R8) 
    {
        writeMipMaps({uint32_t(image.width), uint32_t(image.height), 1}, 1, 0, 1, image.pixels->data());
    }

    if (image.type == Image::Type::RG8)
    {
        assert(image.bytesPerPixel == 2 && "Image::Type::RG8 requires 2 bytes per pixel");
        assert(m_textureDesc.format == WGPUTextureFormat_RG8Unorm && "Texture::Format::RG requires WGPUTextureFormat_RG8Unorm");

        writeMipMaps({uint32_t(image.width), uint32_t(image.height), 1}, 2, 0, 1, image.pixels->data());
    }

    if (image.type == Image::Type::RGB)
    {
        writeMipMaps({uint32_t(image.width), uint32_t(image.height), 1}, image.bytesPerPixel, 0, 10, image.pixels->data());
    }

    if (image.type == Image::Type::RGBA)
    {
        writeMipMaps({uint32_t(image.width), uint32_t(image.height), 1}, 4, 0, 10, image.pixels->data());
    }
}

void Texture::setCubemap(const Cubemap& cubemap)
{
    for (uint32_t i = 0; i < 6; ++i)
    {
        const Image& image = cubemap.faces(i);
        if (image.bytesPerPixel == 4)
        {
            writeMipMaps({uint32_t(image.width), uint32_t(image.height), 6}, image.bytesPerPixel, i, 10, image.pixels->data());
        }
        else
        {
            const Image rgbaImage = image.toRGBA();
            writeMipMaps({uint32_t(rgbaImage.width), uint32_t(rgbaImage.height), 6}, rgbaImage.bytesPerPixel, i, 10, rgbaImage.pixels->data());
        }
    }
}

uint32_t Texture::mipLevelCount() const
{
    return m_textureDesc.mipLevelCount;
}

const WGPUTextureView& Texture::getTextureView() const
{
    return m_textureView;
}
