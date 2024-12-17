#include "viewport.h"

using namespace glm;

Viewport::Viewport(Scheduler& scheduler_, Gpu &gpu, RenderTarget &renderTarget_)
    : scheduler     (scheduler_)
    , renderTarget  (renderTarget_)
    , renderPass    (gpu, renderTarget_)
{
    scheduler_.viewports.emplace_back(this);
}

Viewport::~Viewport()
{
    auto it = std::find(scheduler.viewports.begin(), scheduler.viewports.end(), this);
    scheduler.viewports.erase(it);
}

void Viewport::render()
{
    if (camera == nullptr || light == nullptr) return; // Nothing to render without a camera

    mat4 worldToLight = light->getSpace().fromRoot;

    renderTarget.beginRender();

    vec2 size               = renderTarget.getSize();
    Space projectionSpace   = camera->getProjectionSpace(size.x / size.y);

    vec3 lightpos = light->getSpace().pos(vec3(0.0), Space());

    RenderPass::RenderParams params {
        .lightPosWorld  = lightpos,
        .projection     = camera->getSpace().to(projectionSpace),
        .view           = camera->getSpace().fromRoot
    };

    renderPass.renderPre(params);
    renderPass.render   (renderables);

    renderTarget.endRender();
}

void Viewport::attachCamera(const CameraObject& camera_)
{
    camera = &camera_;
}

void Viewport::attachRenderable(const Renderable& renderable)
{
    renderables.emplace_back(&renderable);
}

void Viewport::attachLight(const Object &light_)
{
    light = &light_;
}