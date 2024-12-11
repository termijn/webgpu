#include "viewport.h"

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
    renderTarget.beginRender();
    renderPass.render();
    renderTarget.endRender();
}
