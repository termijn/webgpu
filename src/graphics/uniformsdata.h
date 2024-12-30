#pragma once

#include <glm/glm.hpp>

struct FrameData
{
    glm::mat4 view                  = glm::mat4(1.0);
    glm::mat4 projection            = glm::mat4(1.0);
    glm::mat4 shadowViewProjection  = glm::mat4(1.0);
    glm::vec4 viewPositionWorld     = glm::vec4(0.0);
    glm::vec4 lightPositionWorld    = glm::vec4(0.0);
    uint32_t  nrPoissonSamples      = 0;
    uint32_t  hasEnvironmentMap     = 0;
    uint32_t  mipLevelCount         = 0;
    uint32_t  padding[1]            = {0};

    bool operator==(const FrameData& other) const;
    bool operator!=(const FrameData& other) const;
};

struct ModelData
{
    glm::mat4 model                  = glm::mat4(1.0);
    glm::mat4 modelInverseTranspose  = glm::mat4(1.0);
    glm::vec4 baseColorFactor        = glm::vec4(1.0);

    uint32_t  hasBaseColorTexture     = 0;
    uint32_t  hasOcclusionTexture     = 0;
    uint32_t  hasNormalTexture        = 0;
    uint32_t  hasEmissiveTexture      = 0;
    uint32_t  hasMetallicRoughnessTexture = 0;
    uint32_t  padding[3]              = {0, 0, 0};

    bool operator==(const ModelData& other) const;
    bool operator!=(const ModelData& other) const;

};


struct FrameDataShadow
{
    glm::mat4 view                  = glm::mat4(1.0);
    glm::mat4 projection            = glm::mat4(1.0);

    bool operator==(const FrameDataShadow& other) const;
    bool operator!=(const FrameDataShadow& other) const;
};

struct ModelDataShadow
{
    glm::mat4 model                  = glm::mat4(1.0);

    bool operator==(const ModelDataShadow& other) const;
    bool operator!=(const ModelDataShadow& other) const;

};
