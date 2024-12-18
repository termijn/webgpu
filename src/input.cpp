#include "input.h"
#include "viewport.h"

using namespace glm;

Input::Input(Viewport &viewport_)
    : viewport(viewport_)
{
    viewport.m_inputs.emplace_back(this);
    button = MouseButton::Left;
}

Input::~Input()
{
    auto it = std::find(viewport.m_inputs.begin(), viewport.m_inputs.end(), this);
    viewport.m_inputs.erase(it);
}

bool Input::query(const vec3 &position, MouseButton button_)
{
    return button == button_;
}

void Input::begin(const vec3 &position)
{
    beginPos = position;
    lastDelta = vec3(0.0);
    lastPos = position;
}

void Input::move(const vec3 &position)
{
    lastDelta = lastPos - position;
    lastPos = position;
}

void Input::end(const vec3 &position)
{
    lastDelta = lastPos - position;
    lastPos = position;
}

void Input::mouseWheel(int direction)
{
}

void Input::animate(double t)
{
}

bool Input::animate()
{
    return false;
}

void Input::startAnimate(double duration)
{
    animationDuration   = duration;
    animationStartTime  = time;
}

void Input::startAnimate()
{
    animationDuration   = std::numeric_limits<double>::max();
    animationStartTime  = time;
}

double inverseQuadratic(double t)
{
    return 1.0 - (1.0 - t) * (1.0 - t);
}

double linear(double t)
{
    return t;
}

double getSigmoid(double x, double power) noexcept
{
    x = (x*2.0)-1.0;

    double value = (1.0 / (1.0 + std::exp(-power * x  ))) - 0.5;
    value       /= (1.0 / (1.0 + std::exp(-power * 1.0))) - 0.5;

    return (value + 1.0) / 2.0;
}

double sigmoidSlow(double x)
{
    return getSigmoid(x, 4.0);
}

double sigmoidFast(double x)
{
    return getSigmoid(x, 8.0);
}

void Input::animateTick(double seconds)
{
    time = seconds;
    if (!animate() && time >= animationStartTime && time <= (animationStartTime + animationDuration))
    {
        double t = (time - animationStartTime) / animationDuration;
        animate(inverseQuadratic(t));
    }
}

vec3 Input::delta() const
{
    return lastPos - beginPos;
}

vec3 Input::deltaRelative() const
{
    return lastDelta;
}
