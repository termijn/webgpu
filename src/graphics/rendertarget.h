#pragma once

#include <webgpu/webgpu.h>

#define SDL_MAIN_HANDLED
#include "sdl2webgpu.h"
#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <string>

#include "gpu.h"

class RenderTarget
{
friend class RenderPass;

public:
    virtual void   beginRender() = 0;
    virtual void   endRender()   = 0;

    virtual glm::vec2   getSize() const = 0;
    
protected:
    WGPUTextureFormat m_surfaceFormat;

    virtual WGPUTextureView getNextTextureView() = 0;
};

class WindowTarget: public RenderTarget
{
public:
    WindowTarget(Gpu& gpu);
    virtual ~WindowTarget();

    void        beginRender()   override;
    void        endRender()     override;
    glm::vec2   getSize() const override;

private:
    SDL_Window*     window      = nullptr;
    WGPUSurface     surface     = nullptr;
    WGPUTextureView targetView  = nullptr;

    WGPUTextureView getNextTextureView() override;
};
