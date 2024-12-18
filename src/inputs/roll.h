#pragma once

#include <glm/glm.hpp>

#include "viewport.h"
#include "input.h"
#include "object.h"

class RollInput: public Input
{
public:
    RollInput(Viewport& viewport, Object& object_);
    
    void begin  (const glm::vec3& position) override;
    void move   (const glm::vec3& position) override;
    void end    (const glm::vec3& position) override;

    void mouseWheel(int direction) override;

    bool animate() override;

private: 
    Object&     object;

    glm::vec2   rotation = glm::vec2(0.0);
    float       zoomFactor  = 0.0;

    

    void roll(const glm::vec2& rotation);
    void zoom(float zoom);

};