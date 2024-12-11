#pragma once

#include <memory>
#include "renderable.h"
#include "mesh.h"

class RenderableObject
{
public:
    RenderableObject(const Object& parent);
    virtual ~RenderableObject();

    Object&     getObject();
    Renderable& getRenderable();

private:
    Object      object;
    Renderable  renderable;
};

class Scene
{

public:

    RenderableObject& newObject(const Object& parent);
    const std::vector<std::unique_ptr<RenderableObject>>& all() const;

    Box getBox();

private:
    std::vector<std::unique_ptr<RenderableObject>> renderables;
};