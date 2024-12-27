#pragma once

#include "graphics/rendertarget.h"
#include "graphics/renderpass.h"
#include "graphics/shadowpass.h"
#include "scheduler.h"
#include "renderable.h"
#include "input.h"
#include "scene.h"

// The viewport renders to a render target (either a window or a canvas).
// It handles mouse events and connects the graphics layer. 
class Viewport
{
friend class Scheduler;
friend class Input;

public:
    Viewport(Scheduler& scheduler, Gpu& gpu, RenderTarget& renderTarget, const Scene& scene);
    virtual ~Viewport();

    void attachCamera       (const CameraObject& camera);
    void attachRenderable   (const Renderable&   renderable);
    void attachLight        (const LightObject&  light);

    void render();

protected:
    void mouseDown  (MouseButton button, const glm::vec3& position);
    void mouseMove  (const glm::vec3& position);
    void mouseUp    (MouseButton button, const glm::vec3& position);
    void mouseWheel (int direction);

private:
    Scheduler&      m_scheduler;
    Gpu&            m_gpu;
    RenderTarget&   m_renderTarget;
    const Scene&    m_scene;
    
    const CameraObject*     m_camera  = nullptr;
    const LightObject*      m_light   = nullptr;
    
    std::vector<const Renderable*>  m_renderables;
    std::vector<Input*>             m_inputs;
    Input*                          m_activeInput = nullptr;
    MouseButton                     m_pressedButtons      = MouseButton::None;

    DepthTarget m_depthTarget;
    ShadowPass  m_shadowPass;
    RenderPass  m_renderPass;

    Viewport (const Viewport&)              = delete;
    Viewport& operator= (const Viewport&)   = delete;

};