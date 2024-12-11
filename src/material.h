#pragma once

#include <optional>
#include <glm/glm.hpp>
#include "image.h"
#include "cubemap.h"

struct TextureTransforms
{
    glm::vec2   offset      = glm::vec2(0.0, 0.0);
    glm::vec2   scale       = glm::vec2(1.0, 1.0);
    float       rotation    = 0.0;

    int uvSet = -1;

};
class Material
{
public:
    glm::vec3   albedo      = glm::vec3(0.0, 0.5, 0.6);
    float       roughness   = 0.6f;
    float       metallic    = 1.0f;

    bool        castsShadow = true;
    bool        shaded      = true;

    std::optional<Image>    baseColorTexture;
    std::optional<Image>    metallicRoughness;
    std::optional<Image>    emissive;
    std::optional<Image>    normalMap;
    std::optional<Image>    occlusion;

    std::optional<Cubemap*>  reflectionMap;
    
    std::optional<std::vector<glm::vec2>>   uvSet;
    std::optional<std::vector<glm::vec2>>   uvSetNormalMap;
    float normalMapScale = 1.0;

    TextureTransforms baseColorTransforms           = TextureTransforms();
    TextureTransforms metallicRoughnessTransforms   = TextureTransforms();

};