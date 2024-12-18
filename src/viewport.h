#pragma once

#include "graphics/rendertarget.h"
#include "graphics/renderpass.h"
#include "scheduler.h"
#include "renderable.h"
#include "input.h"

class Viewport
{
friend class Scheduler;
friend class Input;

public:
    Viewport(Scheduler& scheduler, Gpu& gpu, RenderTarget& renderTarget);
    virtual ~Viewport();

    void attachCamera       (const CameraObject& camera);
    void attachRenderable   (const Renderable&   renderable);
    void attachLight        (const Object&       light);

    void render();

protected:
    void mouseDown  (MouseButton button, const glm::vec3& position);
    void mouseMove  (const glm::vec3& position);
    void mouseUp    (MouseButton button, const glm::vec3& position);
    void mouseWheel (int direction);

private:
    Scheduler&      scheduler;
    RenderTarget&   renderTarget;
    
    const CameraObject*     camera  = nullptr;
    const Object*           light   = nullptr;
    
    std::vector<const Renderable*>  renderables;
    std::vector<Input*>             m_inputs;
    Input*                          m_activeInput = nullptr;
    MouseButton                     m_pressedButtons      = MouseButton::None;

    RenderPass      renderPass;

    Viewport (const Viewport&)              = delete;
    Viewport& operator= (const Viewport&)   = delete;

};