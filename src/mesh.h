#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

class Box
{
public:
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::min());

    void expand(const glm::vec3& point);

    glm::vec3 center() const;
};

class Vertex
{
public:
    glm::vec4 position;
    glm::vec4 normal;
    glm::vec2 uv;
    glm::vec4 tangent    = glm::vec4(0.0);
    glm::vec4 bitangent  = glm::vec4(0.0);

};

class Mesh
{
public:
    Mesh();
    Mesh(const Mesh& rhs);

    Mesh& operator=(const Mesh& rhs);

    std::vector<Vertex>&         vertices();
    std::vector<glm::u32vec3>&   indices();

    std::vector<Vertex>&         vertices() const;
    std::vector<glm::u32vec3>&   indices() const;

    Box boundingBox;

    void generateNormals();
    void generateBoundingBox();
    void generateTangentVectors();

    void cube       (float size);
    void noisySphere(float radius, int rings, int sectors, float noiseAmplitude);
    void knot       (float radius, float tubeRadius, int segments, int sides);
    void sphere     (float radius, int rings, int sectors);
    void quad       ();

private:
    std::shared_ptr<std::vector<Vertex>>        vertexData  = std::make_shared<std::vector<Vertex>>();
    std::shared_ptr<std::vector<glm::u32vec3>>  indicesData = std::make_shared<std::vector<glm::u32vec3>>();

    void copy(const Mesh& rhs);

};