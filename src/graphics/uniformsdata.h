#pragma once

#include <glm/glm.hpp>

struct FrameData
{
    glm::mat4 view                  = glm::mat4(1.0);
    glm::mat4 projection            = glm::mat4(1.0);
    glm::vec4 viewPositionWorld     = glm::vec4(0.0);
    glm::vec4 lightPositionWorld    = glm::vec4(0.0);

    bool operator==(const FrameData& other) const;
    bool operator!=(const FrameData& other) const;
};

struct ModelData
{
    glm::mat4 model                  = glm::mat4(1.0);
    glm::mat4 modelInverseTranspose  = glm::mat4(1.0);

    bool operator==(const ModelData& other) const;
    bool operator!=(const ModelData& other) const;

};