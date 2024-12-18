#include "rendertarget.h"
#include <iostream>

using namespace glm;

WindowTarget::WindowTarget(Gpu& gpu)
    : m_depthTexture(gpu, Texture::Params {.usage = Texture::Usage::RenderAttachment, .sampleCount = 4, .format = Texture::Format::Depth })
    , m_msaaTexture (gpu, Texture::Params {.usage = Texture::Usage::RenderAttachment, .sampleCount = 4, .format = Texture::Format::RGBA })
{
    int windowFlags = SDL_WINDOW_RESIZABLE;
    window  = SDL_CreateWindow("WebGPU renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, windowFlags);
    surface = sdl2GetWGPUSurface(gpu.getInstance(), window);

    WGPUSurfaceCapabilities capabilities;
    capabilities.nextInChain = nullptr;
    wgpuSurfaceGetCapabilities(surface, gpu.m_adapter, &capabilities);
    m_surfaceFormat = capabilities.formats[0];

    surfaceConfig.nextInChain      = nullptr;
    surfaceConfig.width            = 640;
    surfaceConfig.height           = 480;
    surfaceConfig.format           = m_surfaceFormat;
    surfaceConfig.viewFormatCount  = 0;
    surfaceConfig.viewFormats      = nullptr;
    surfaceConfig.usage            = WGPUTextureUsage_RenderAttachment;
    surfaceConfig.device           = gpu.m_device;
    surfaceConfig.presentMode      = WGPUPresentMode_Fifo;
    surfaceConfig.alphaMode        = WGPUCompositeAlphaMode_Auto;

    wgpuSurfaceConfigure(surface, &surfaceConfig);
}

WindowTarget::~WindowTarget()
{
    wgpuSurfaceUnconfigure(surface);
    surface = nullptr;
    window  = nullptr;
}

void WindowTarget::beginRender()
{
    glm::vec2 size = getSize();
    surfaceConfig.width = size.x;
    surfaceConfig.height = size.y;
    wgpuSurfaceConfigure(surface, &surfaceConfig);

    m_msaaTexture.setSize(size);
    m_depthTexture.setSize(size);
}

void WindowTarget::endRender()
{
    wgpuTextureViewRelease(targetView);
    wgpuSurfacePresent(surface);
}

glm::vec2 WindowTarget::getSize() const
{
    int winWidth = 512, winHeight = 512;
    SDL_GetWindowSizeInPixels(window, &winWidth, &winHeight);
    return vec2(winWidth, winHeight);
}

WGPUTextureView WindowTarget::getNextTextureView()
{
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success)
        return nullptr;

    WGPUTextureViewDescriptor viewDescriptor {};
    viewDescriptor.nextInChain = nullptr;
    viewDescriptor.label = "Surface texture view";
    viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
    viewDescriptor.dimension = WGPUTextureViewDimension_2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = WGPUTextureAspect_All;
    
    targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);
    #ifndef WEBGPU_BACKEND_WGPU
    // We no longer need the texture, only its view
    // (NB: with wgpu-native, surface textures must not be manually released)
    wgpuTextureRelease(surfaceTexture.texture);
    #endif // WEBGPU_BACKEND_WGPU
    return targetView;
}

WGPUTextureView WindowTarget::getDepthTextureView()
{
    return m_depthTexture.getTextureView();
}

WGPUTextureView WindowTarget::getMsaaTextureView()
{
    return m_msaaTexture.getTextureView();
}
