#include "resourcepool.h"

#include <iostream>
#include <fstream>

#include "graphics/gpu.h"

ResourcePool::ResourcePool(Gpu& gpu)
    : m_gpu(gpu)
{

}
ResourcePool::~ResourcePool() = default;

// Texture &ResourcePool::get(const Image *image, Texture::Interpolation interpolation)
// {
//     if (!poolTextures.contains(image->getPixels()))
//     {
//         poolTextures.emplace(image->getPixels(), Texture());
//         poolTextures[image->getPixels()].setImage(*image, interpolation);
//     }
//     return poolTextures[image->getPixels()];
// }

VertexBuffer &ResourcePool::get(const Renderable *renderable)
{
    auto it = poolVertexBuffers.find(&renderable->mesh.vertices());
    if (it == poolVertexBuffers.end()) 
    {
        auto [newIt, inserted] = poolVertexBuffers.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(&renderable->mesh.vertices()),
            std::forward_as_tuple(m_gpu)
        );

        if (inserted) {
            newIt->second.setMesh(&renderable->mesh);
        }
        it = newIt;
    }
    return it->second;
}

// CubemapTexture &ResourcePool::get(const Cubemap *cubemap)
// {
//     if (!poolCubemaps.contains(cubemap))
//     {
//         poolCubemaps.emplace(std::piecewise_construct,
//                           std::forward_as_tuple(cubemap),
//                           std::tuple<>());
//         poolCubemaps[cubemap].setCubemap(*cubemap);
//     }
//     return poolCubemaps[cubemap];
// }
