#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>

class Space
{
public:
    glm::mat4 toRoot    = glm::mat4(1.0);
    glm::mat4 fromRoot  = glm::mat4(1.0);

    glm::mat4 to  (const Space& target) const;
    glm::vec3 pos (const glm::vec3& position,     const Space& targetSpace) const;
    glm::vec3 dir (const glm::vec3& direction,    const Space& targetSpace) const;

    static glm::vec3 dir(const glm::vec3& direction,   const Space& from, const Space& to);
    static glm::vec3 pos(const glm::vec3& pos,         const Space& from, const Space& to);

};

class Object
{
public:
    Object();
    explicit Object(const Object& parent);

    virtual ~Object();

    void adopt  (const Object& parent);
    void orphan ();

    // Sets a new transformation to parent
    void        setTransform(const glm::mat4& toParent);
    glm::mat4   getTransform() const;

    // Sets the origin at from, aligns the negative z-axis with to-from, respecting the up vector for the y-axis.
    // Positions and directions are defined with respect to the parent Object
    void lookAt(const glm::vec3& from, const glm::vec3& to, const glm::vec3& up);

    Space getSpace() const;
    Space getParentSpace() const;

protected:
    virtual void updateTransforms() const;

private:
    const Object*   parent      = nullptr;
    
    mutable Space   space;
    glm::mat4       toParent    = glm::mat4(1.0);

    mutable std::vector<const Object*> children;

    void removeFromParent();

    Object&  operator= (const Object&)   = delete;
    Object&& operator= (const Object&&)   = delete;

};

class CameraObject: public Object
{
public:
    explicit CameraObject(const Object& parent);

    void setPerspective(float fov, float near, float far);

    const Space getProjectionSpace(float aspectRatio) const;

private:
    float fov;
    float near; 
    float far;

    CameraObject& operator= (const CameraObject&)   = delete;

};