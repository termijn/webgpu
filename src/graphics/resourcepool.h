#pragma once

#include "renderable.h"
#include "image.h"
#include "cubemap.h"

#include "graphics/vertexbuffer.h"
#include "graphics/texture.h"
//#include "renderer/cubemaptexture.h"

#include <map>
#include <string>

class Gpu;

class ResourcePool
{
public:
    ResourcePool(Gpu& gpu);
    ~ResourcePool();

    Texture&        get(const Image* image);
    VertexBuffer&   get(const Renderable* renderable);
  //  CubemapTexture& get(const Cubemap* cubemap);

private:
    Gpu& m_gpu;

    std::map<const std::vector<Vertex>*, VertexBuffer>   poolVertexBuffers;
    std::map<const uint8_t*,    Texture>                 m_poolTextures;
//    std::map<const Cubemap*, CubemapTexture>             poolCubemaps;

    ResourcePool            (const ResourcePool&)   = delete;
    ResourcePool& operator= (const ResourcePool&)   = delete;

};
