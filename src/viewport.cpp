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

    vec4 lightpos = vec4(light->getSpace().pos(vec3(0.0), Space()), 1.0);
    
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
