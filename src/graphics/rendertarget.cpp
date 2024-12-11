#include "rendertarget.h"

using namespace glm;

WindowTarget::WindowTarget(Gpu& gpu)
{
    int windowFlags = SDL_WINDOW_RESIZABLE;
    window  = SDL_CreateWindow("WebGPU renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, windowFlags);
    surface = sdl2GetWGPUSurface(gpu.getInstance(), window);

    WGPUSurfaceCapabilities capabilities;
    capabilities.nextInChain = nullptr;
    wgpuSurfaceGetCapabilities(surface, gpu.m_adapter, &capabilities);
    m_surfaceFormat = capabilities.formats[0];

    WGPUSurfaceConfiguration config {};
    config.nextInChain      = nullptr;
    config.width            = 640;
    config.height           = 480;
    config.format           = m_surfaceFormat;
    config.viewFormatCount  = 0;
    config.viewFormats      = nullptr;
    config.usage            = WGPUTextureUsage_RenderAttachment;
    config.device           = gpu.m_device;
    config.presentMode      = WGPUPresentMode_Fifo;
    config.alphaMode        = WGPUCompositeAlphaMode_Auto;

    wgpuSurfaceConfigure(surface, &config);
}

WindowTarget::~WindowTarget()
{
    wgpuSurfaceUnconfigure(surface);
    surface = nullptr;
    window  = nullptr;
}

void WindowTarget::beginRender()
{
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
