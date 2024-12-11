#include "scene.h"

using namespace glm;

RenderableObject& Scene::newObject(const Object& parent)
{
    return *renderables.emplace_back(std::make_unique<RenderableObject>(parent));
}

const std::vector<std::unique_ptr<RenderableObject>>& Scene::all() const
{
    return renderables;
}

Box Scene::getBox()
{
    Box box;
    for(auto& renderableObject : renderables)
    {
        Box localBox = renderableObject->getRenderable().mesh.boundingBox;
        vec3 minRoot = Space::pos(localBox.min, renderableObject->getObject().getSpace(), Space());
        vec3 maxRoot = Space::pos(localBox.max, renderableObject->getObject().getSpace(), Space());
        box.expand(minRoot);
        box.expand(maxRoot);
    }
    return box;
}

RenderableObject::RenderableObject(const Object& parent)
    : object(parent)
    , renderable(object)
{
}

RenderableObject::~RenderableObject() 
{

}

Object &RenderableObject::getObject()
{
    return object;
}

Renderable &RenderableObject::getRenderable()
{
    return renderable;
}
