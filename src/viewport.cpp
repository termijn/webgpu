#include "viewport.h"

using namespace glm;

Viewport::Viewport(Scheduler& scheduler, Gpu &gpu, RenderTarget &renderTarget)
    : m_scheduler     (scheduler)
    , m_gpu           (gpu)
    , m_renderTarget  (renderTarget)
    , m_depthTarget   (gpu)
    , m_shadowPass    (gpu, m_depthTarget)
    , m_renderPass    (gpu, renderTarget, m_depthTarget)
{
    m_scheduler.viewports.emplace_back(this);
}

Viewport::~Viewport()
{
    auto it = std::find(m_scheduler.viewports.begin(), m_scheduler.viewports.end(), this);
    m_scheduler.viewports.erase(it);
}

void Viewport::render()
{
    if (m_camera == nullptr || m_light == nullptr) return; // Nothing to render without a camera

    m_gpu.beginRenderJob();

    vec2 size               = m_renderTarget.getSize();
    Space projectionSpace   = m_camera->getProjectionSpace(size.x / size.y);

    m_renderTarget.beginRender();
    m_depthTarget.beginRender();
    ShadowPass::RenderParams shadowParams { 
        .view       = m_light->getView(),
        .projection = m_light->getProjection()
    };
    m_shadowPass.renderPre  (shadowParams);
    m_shadowPass.render     (m_renderables);

    vec4 lightpos = vec4(m_light->getSpace().pos(vec3(0.0), Space()), 1.0);
    
    RenderPass::RenderParams params {
        .lightPosWorld  = lightpos,
        .projection     = m_camera->getSpace().to(projectionSpace),
        .view           = m_camera->getSpace().fromRoot,
        .shadowViewProjection = m_light->getProjection() * m_light->getView()
    };

    m_renderPass.renderPre(params);
    m_renderPass.render   (m_renderables);

    m_gpu.submitRenderJob();

    m_renderTarget.endRender();
    m_depthTarget.endRender();
}

void Viewport::attachCamera(const CameraObject& camera_)
{
    m_camera = &camera_;
}

void Viewport::attachRenderable(const Renderable& renderable)
{
    m_renderables.emplace_back(&renderable);
}

void Viewport::attachLight(const LightObject &light_)
{
    m_light = &light_;
}

void Viewport::mouseDown(MouseButton button, const glm::vec3& position)
{
    std::cout << "mouseDown" << std::endl;
    m_pressedButtons = static_cast<MouseButton>(static_cast<unsigned int>(m_pressedButtons) | static_cast<unsigned int>(button));

    if (m_activeInput != nullptr)
        m_activeInput->end(position);

    m_activeInput = nullptr;

    for(auto input: m_inputs)
    {
        if (input->query(position, m_pressedButtons))
        {
            m_activeInput = input;
            m_activeInput->begin(position);
        }
    }
}

void Viewport::mouseMove(const glm::vec3 &position)
{
    std::cout << "mouseMove" << std::endl;

    if (m_activeInput == nullptr)
    {
        for(auto input: m_inputs)
        {
            if (input->query(position, m_pressedButtons))
                m_activeInput = input;
        }
    } 
    else 
    {
        m_activeInput->move(position);
    }
}

void Viewport::mouseUp(MouseButton button, const glm::vec3& position)
{
    std::cout << "mouseUp" << std::endl;
    m_pressedButtons = static_cast<MouseButton>(static_cast<unsigned int>(m_pressedButtons) & ~static_cast<unsigned int>(button));

    if (m_activeInput != nullptr)
    {
        m_activeInput->end(position);
        m_activeInput = nullptr;
    }
}

void Viewport::mouseWheel(int direction)
{
    for(auto input: m_inputs)
    {
        input->mouseWheel(direction);
    }
}
