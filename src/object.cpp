#include "object.h"
#include <glm/gtc/matrix_transform.hpp> 

#include <iostream>

using namespace glm;

Object::Object()
{
}

Object::Object(const Object &parent_)
    : parent(&parent_)
{
    assert(&parent_ != this);

    parent_.children.push_back(this);
    updateTransforms();
}

Object::~Object()
{
    orphan();
}

void Object::adopt(const Object& parent_)
{
    removeFromParent();

    parent = &parent_;
    parent->children.push_back(this);

    updateTransforms();
}

void Object::orphan()
{
   removeFromParent();
   updateTransforms();
}

void Object::setTransform(const mat4 &toParent_)
{
    toParent = toParent_;
    updateTransforms();
}

glm::mat4 Object::getTransform() const
{
    return toParent;
}

void Object::lookAt(const vec3 &from, const vec3 &to, const vec3 &up)
{
    mat4 fromParent = glm::lookAt(from, to, up);
    setTransform(inverse(fromParent));
}

Space Object::getSpace() const
{
    return space;
}

Space Object::getParentSpace() const
{
    return parent->getSpace();
}

void Object::updateTransforms() const
{
    if (parent == nullptr)
    {
        space.toRoot      = toParent;
        space.fromRoot    = inverse(toParent);
    }
    else
    {
        space.toRoot      = parent->space.toRoot * toParent;
        space.fromRoot    = inverse(space.toRoot);
    }

    for(auto child : children)
        child->updateTransforms();
}

void Object::removeFromParent()
{
    std::cout << "removeFromParent" << std::endl;
    if (parent != nullptr)
    {
        std::cout << "removeFromParent - parent != nullptr" << std::endl;
        auto it = std::find(parent->children.begin(), parent->children.end(), this);
        if (it != parent->children.end())
            parent->children.erase(it);
    }
    parent = nullptr;
}

mat4 Space::to(const Space &target) const
{
    return target.fromRoot * toRoot;
}

vec3 Space::transformPos(const vec3 &position, const Space &targetSpace) const
{
    vec4 result = to(targetSpace) * vec4(position, 1.0);
    result /= result.w;
    return result;
}

vec3 Space::transformDir(const vec3 &direction, const Space &targetSpace) const
{
    return to(targetSpace) * vec4(direction, 0.0);
}

glm::vec3 Space::dir(const glm::vec3 &direction, const Space &from, const Space &to)
{
    return from.transformDir(direction, to);
}

glm::vec3 Space::pos(const glm::vec3 &pos, const Space &from, const Space &to)
{
        return from.transformDir(pos, to);
}

CameraObject::CameraObject(const Object &parent)
    : Object(parent)
{
}

void CameraObject::setPerspective(float fov_, float near_, float far_)
{
    fov     = fov_;
    near    = near_;
    far     = far_;
    updateTransforms();
}

const Space CameraObject::getProjectionSpace(float aspectRatio) const
{
    mat4 projection = perspective(fov, aspectRatio, near, far);

    Space projectionSpace;
    projectionSpace.fromRoot  = projection * getSpace().fromRoot;
    projectionSpace.toRoot    = inverse(projectionSpace.fromRoot);

    return projectionSpace;
}
