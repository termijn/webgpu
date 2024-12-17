#pragma once

#include "graphics/rendertarget.h"
#include "graphics/renderpass.h"
#include "scheduler.h"
#include "renderable.h"

class Viewport
{
public:
    Viewport(Scheduler& scheduler, Gpu& gpu, RenderTarget& renderTarget);
    virtual ~Viewport();

    void attachCamera       (const CameraObject& camera);
    void attachRenderable   (const Renderable&   renderable);
    void attachLight        (const Object&       light);

    void render();

private:
    Scheduler&      scheduler;
    RenderTarget&   renderTarget;
    
    const CameraObject*     camera  = nullptr;
    const Object*           light   = nullptr;
    
    std::vector<const Renderable*>  renderables;

    RenderPass      renderPass;

    Viewport (const Viewport&)              = delete;
    Viewport& operator= (const Viewport&)   = delete;

};