#pragma once

#include <glm/glm.hpp>
#include "object.h"
#include "mesh.h"
#include "material.h"

class Renderable
{
public:
    Renderable(Object& object);
    ~Renderable();

    Object*     object;
    Material    material;
    Mesh        mesh;

    Renderable (const Renderable&)              = delete;
    Renderable& operator= (const Renderable&)   = delete;

    Renderable(Renderable&& other) noexcept;
    Renderable& operator=(Renderable&& other) noexcept;

};
