#include "renderable.h"

Renderable::Renderable(Object &object_)
    : object(&object_)
{
}

Renderable::~Renderable()
{
}


Renderable::Renderable(Renderable &&other) noexcept
    : object     (other.object)
    , material   (std::move(other.material))
    , mesh       (std::move(other.mesh))
{
}

Renderable &Renderable::operator=(Renderable &&other) noexcept
{
    if (this != &other) 
    {
        object      = std::move(other.object);
        material    = std::move(other.material);
        mesh        = std::move(other.mesh);
    }
    return *this;
}
