#include "resourcepool.h"

#include <iostream>
#include <fstream>

#include "graphics/gpu.h"

ResourcePool::ResourcePool(Gpu& gpu)
    : m_gpu(gpu)
{

}
ResourcePool::~ResourcePool() 
{
    
};

Texture& ResourcePool::get(const Image* image, bool srgb)
{
    auto it = m_poolTextures.find(image->getPixels());
    if (it == m_poolTextures.end()) 
    {
        Texture::Params params
        {
            .format = srgb ? Texture::Format::RGBAsrgb : Texture::Format::RGBA,
            .sampleCount = 1,
            .usage = Texture::Usage::CopySrcTextureBinding
        };
        auto [newIt, inserted] = m_poolTextures.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(image->getPixels()),
            std::forward_as_tuple(m_gpu, params)
        );

        if (inserted) {
            newIt->second.setImage(*image);
        }
        it = newIt;
    }
    return it->second;
}

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

Texture& ResourcePool::get(const Cubemap* cubemap)
{
    auto it = m_poolCubemaps.find(cubemap);
    if (it == m_poolCubemaps.end()) 
    {
        Texture::Params params
        {
            .format = Texture::Format::RGBAsrgb,
            .sampleCount = 1,
            .usage = Texture::Usage::CopySrcTextureBinding
        };
        auto [newIt, inserted] = m_poolCubemaps.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(cubemap),
            std::forward_as_tuple(m_gpu, params)
        );

        if (inserted) {
            newIt->second.setCubemap(*cubemap);
        }
        it = newIt;
    }
    return it->second;
}
