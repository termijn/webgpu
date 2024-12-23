#pragma once

#include <webgpu/webgpu.h>

#define SDL_MAIN_HANDLED
#include "sdl2webgpu.h"
#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <string>

#include "gpu.h"
#include "texture.h"

class RenderTarget
{
friend class RenderPass;
friend class ShadowPass;

public:
    virtual void   beginRender() = 0;
    virtual void   endRender()   = 0;

    virtual glm::vec2   getSize() const = 0;

    virtual WGPUTextureView getNextTextureView() = 0;
    virtual WGPUTextureView getDepthTextureView() = 0;
    virtual WGPUTextureView getMsaaTextureView() = 0;

protected:
    WGPUTextureFormat m_surfaceFormat;

};

class WindowTarget: public RenderTarget
{
public:
    WindowTarget(Gpu& gpu);
    virtual ~WindowTarget();

    void        beginRender()   override;
    void        endRender()     override;
    glm::vec2   getSize() const override;

    WGPUTextureView getNextTextureView() override;
    WGPUTextureView getDepthTextureView() override;
    WGPUTextureView getMsaaTextureView() override;

    Texture m_depthTexture;
    Texture m_msaaTexture;

private:
    SDL_Window*     window      = nullptr;
    WGPUSurface     surface     = nullptr;
    WGPUTextureView targetView  = nullptr;
    WGPUSurfaceConfiguration surfaceConfig = {};

};

class DepthTarget: public RenderTarget
{
friend class ShadowPass;

public:
    DepthTarget(Gpu& gpu);
    virtual ~DepthTarget();

    void setSize(glm::vec2 size);

    void        beginRender()   override;
    void        endRender()     override;
    glm::vec2   getSize() const override;

    WGPUTextureView getNextTextureView() override;
    WGPUTextureView getDepthTextureView() override;
    WGPUTextureView getMsaaTextureView() override;

    Texture m_depthTexture;

private:
    glm::vec2 m_size = glm::vec2(0, 0);
    WGPUTextureView depthTextureView  = nullptr;

};