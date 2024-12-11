#pragma once

#include "graphics/rendertarget.h"
#include "graphics/renderpass.h"
#include "scheduler.h"

class Viewport
{
public:
    Viewport(Scheduler& scheduler, Gpu& gpu, RenderTarget& renderTarget);
    virtual ~Viewport();

    void render();

private:
    Scheduler&      scheduler;
    RenderTarget&   renderTarget;
    RenderPass      renderPass;
};