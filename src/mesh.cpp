#include "mesh.h"
#include "iostream"
#include <glm/ext/scalar_constants.hpp>

using namespace glm;

Mesh::Mesh()
{

}

Mesh::Mesh(const Mesh& rhs)
{
    copy(rhs);
}

Mesh &Mesh::operator=(const Mesh &rhs)
{
    if (&rhs == this) return *this;
    copy(rhs);
    return *this;
}

std::vector<Vertex> &Mesh::vertices()
{
    return *vertexData;
}

std::vector<glm::u32vec3> &Mesh::indices()
{
    return *indicesData;
}

std::vector<Vertex> &Mesh::vertices() const
{
    return *vertexData;
}

std::vector<glm::u32vec3> &Mesh::indices() const
{
    return *indicesData;
}

void Mesh::generateNormals()
{
    // TODO: not yet implemented
}

void Mesh::generateBoundingBox()
{
    boundingBox.min = glm::vec3(90000000.0f);
    boundingBox.max = glm::vec3(-90000000.0f);

    for(auto& vertex: vertices())
        boundingBox.expand(vertex.position);
}

void Mesh::generateTangentVectors()
{
    if (vertices().empty() || indices().empty()) return;

    for (const auto& index : indices())
    {
        Vertex& v0 = vertices()[index.x];
        Vertex& v1 = vertices()[index.y];
        Vertex& v2 = vertices()[index.z];

        glm::vec3 pos1 = glm::vec3(v0.position);
        glm::vec3 pos2 = glm::vec3(v1.position);
        glm::vec3 pos3 = glm::vec3(v2.position);

        glm::vec2 uv1 = v0.uv;
        glm::vec2 uv2 = v1.uv;
        glm::vec2 uv3 = v2.uv;

        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;

        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);

        glm::vec3 bitangent;
        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent = glm::normalize(bitangent);

        v0.tangent += glm::vec4(tangent, 0.0f);
        v1.tangent += glm::vec4(tangent, 0.0f);
        v2.tangent += glm::vec4(tangent, 0.0f);

        v0.bitangent += glm::vec4(bitangent, 0.0f);
        v1.bitangent += glm::vec4(bitangent, 0.0f);
        v2.bitangent += glm::vec4(bitangent, 0.0f);
    }

    // Normalize tangents and apply Gram-Schmidt orthogonalization
    for (auto& vertex : vertices()) 
    {
        glm::vec3 normal    = glm::normalize(glm::vec3(vertex.normal));
        glm::vec3 tangent   = glm::normalize(glm::vec3(vertex.tangent));

        // Orthogonalize tangent
        tangent = glm::normalize(tangent - glm::dot(tangent, normal) * normal);

        // Recalculate the bitangent
        glm::vec3 bitangent = glm::cross(normal, tangent);

        // Check and fix handedness
        float handedness    = (glm::dot(glm::cross(normal, tangent), glm::vec3(vertex.bitangent)) < 0.0f) ? -1.0f : 1.0f;
        vertex.tangent      = glm::vec4(tangent, handedness);
        vertex.bitangent    = glm::vec4(bitangent, 0.0f);
    }
}

void Mesh::cube(float size)
{
    float halfSize = size / 2.0f;

    // Define the vertices() of the cube, each with the appropriate normal for each face
    vertices() = {
        // Back face (-Z)
        { vec4(-halfSize, -halfSize, -halfSize, 1.0f), vec4(0, 0, -1, 0) },
        { vec4( halfSize, -halfSize, -halfSize, 1.0f), vec4(0, 0, -1, 0) },
        { vec4( halfSize,  halfSize, -halfSize, 1.0f), vec4(0, 0, -1, 0) },
        { vec4(-halfSize,  halfSize, -halfSize, 1.0f), vec4(0, 0, -1, 0) },

        // Front face (+Z)
        { vec4(-halfSize, -halfSize,  halfSize, 1.0f), vec4(0, 0, 1, 0) },
        { vec4( halfSize, -halfSize,  halfSize, 1.0f), vec4(0, 0, 1, 0) },
        { vec4( halfSize,  halfSize,  halfSize, 1.0f), vec4(0, 0, 1, 0) },
        { vec4(-halfSize,  halfSize,  halfSize, 1.0f), vec4(0, 0, 1, 0) },

        // Left face (-X)
        { vec4(-halfSize, -halfSize, -halfSize, 1.0f), vec4(-1, 0, 0, 0) },
        { vec4(-halfSize,  halfSize, -halfSize, 1.0f), vec4(-1, 0, 0, 0) },
        { vec4(-halfSize,  halfSize,  halfSize, 1.0f), vec4(-1, 0, 0, 0) },
        { vec4(-halfSize, -halfSize,  halfSize, 1.0f), vec4(-1, 0, 0, 0) },

        // Right face (+X)
        { vec4(halfSize, -halfSize, -halfSize, 1.0f), vec4(1, 0, 0, 0) },
        { vec4(halfSize,  halfSize, -halfSize, 1.0f), vec4(1, 0, 0, 0) },
        { vec4(halfSize,  halfSize,  halfSize, 1.0f), vec4(1, 0, 0, 0) },
        { vec4(halfSize, -halfSize,  halfSize, 1.0f), vec4(1, 0, 0, 0) },

        // Bottom face (-Y)
        { vec4(-halfSize, -halfSize, -halfSize, 1.0f), vec4(0, -1, 0, 0) },
        { vec4( halfSize, -halfSize, -halfSize, 1.0f), vec4(0, -1, 0, 0) },
        { vec4( halfSize, -halfSize,  halfSize, 1.0f), vec4(0, -1, 0, 0) },
        { vec4(-halfSize, -halfSize,  halfSize, 1.0f), vec4(0, -1, 0, 0) },

        // Top face (+Y)
        { vec4(-halfSize, halfSize, -halfSize, 1.0f), vec4(0, 1, 0, 0) },
        { vec4( halfSize, halfSize, -halfSize, 1.0f), vec4(0, 1, 0, 0) },
        { vec4( halfSize, halfSize,  halfSize, 1.0f), vec4(0, 1, 0, 0) },
        { vec4(-halfSize, halfSize,  halfSize, 1.0f), vec4(0, 1, 0, 0) },
    };

    // Define the 12 triangles (6 faces) of the cube using indices()
    indices() = {
        u16vec3{0, 1, 2}, u16vec3{0, 2, 3},   // Back face
        u16vec3{4, 5, 6}, u16vec3{4, 6, 7},   // Front face
        u16vec3{8, 9, 10}, u16vec3{8, 10, 11}, // Left face
        u16vec3{12, 13, 14}, u16vec3{12, 14, 15}, // Right face
        u16vec3{16, 17, 18}, u16vec3{16, 18, 19}, // Bottom face
        u16vec3{20, 21, 22}, u16vec3{20, 22, 23}  // Top face
    };
}

void Mesh::noisySphere(float radius, int rings, int sectors, float noiseAmplitude)
{
    vertices().clear();
    indices().clear();

    // Generate vertices()
    for (int r = 0; r <= rings; ++r) {
        for (int s = 0; s <= sectors; ++s) {
            float phi = pi<float>() * r / rings; // latitude
            float theta = 2.0f * pi<float>() * s / sectors; // longitude

            // Compute the position of the vertex on the sphere
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            // Apply noise
            float noise = ((float)rand() / (float) RAND_MAX) * noiseAmplitude; // Random noise in [0, noiseAmplitude]
            x += noise * (rand() % 2 == 0 ? 1 : -1); // Randomly add or subtract noise
            y += noise * (rand() % 2 == 0 ? 1 : -1);
            z += noise * (rand() % 2 == 0 ? 1 : -1);

            vec4 position(x, y, z, 1.0f);
            vec4 normal = normalize(position); // Normal is the normalized position vector

            vertices().push_back({ position, normal });
        }
    }

    // Generate indices()
    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            int first = (r * (sectors + 1)) + s;
            int second = first + sectors + 1;

            indices().push_back({ static_cast<u16>(first), static_cast<u16>(second), static_cast<u16>(first + 1) });
            indices().push_back({ static_cast<u16>(second), static_cast<u16>(second + 1), static_cast<u16>(first + 1) });
        }
    }
}

void Mesh::knot(float radius, float tubeRadius, int segments, int sides)
{
    vertices().clear();
    indices().clear();

    for (int i = 0; i <= segments; ++i)
    {
        float t = i * 2.0f * pi<float>() / segments;

        // Parametric equations for the knot centerline
        vec3 center = vec3(
            (2.0f + cos(3.0f * t)) * cos(2.0f * t),
            (2.0f + cos(3.0f * t)) * sin(2.0f * t),
            sin(3.0f * t)
        ) * radius;

        // Calculate the derivative (tangent) of the knot for Frenet frame
        vec3 tangent = normalize(vec3(
            -2.0f * sin(2.0f * t) * (2.0f + cos(3.0f * t)) - 6.0f * sin(3.0f * t) * cos(2.0f * t),
            2.0f * cos(2.0f * t) * (2.0f + cos(3.0f * t)) - 6.0f * sin(3.0f * t) * sin(2.0f * t),
            3.0f * cos(3.0f * t)
        ));

        // Find two perpendicular vectors to the tangent to create the Frenet frame
        vec3 bitangent = normalize(cross(tangent, vec3(0.0f, 0.0f, 1.0f)));
        vec3 normal = normalize(cross(tangent, bitangent));

        for (int j = 0; j <= sides; ++j)
        {
            float s = j * 2.0f * pi<float>() / sides;

            // Compute the position of each vertex around the centerline
            vec3 circlePos = cos(s) * normal + sin(s) * bitangent;

            vec4 position = vec4(center + circlePos * tubeRadius, 1.0f); // thickness of the knot
            vec4 vertexNormal = vec4(normalize(circlePos), 0.0f);

            vertices().push_back({position, vertexNormal});
        }
    }

    // Create indices to form quads or triangles between the segments and sides
    for (int i = 0; i < segments; ++i)
    {
        for (int j = 0; j < sides; ++j)
        {
            int nextI = (i + 1) % segments;
            int nextJ = (j + 1) % sides;

            // Define triangles or quads (two triangles per quad)
            indices().push_back(u16vec3(i * (sides + 1) + j, nextI * (sides + 1) + j, i * (sides + 1) + nextJ));
            indices().push_back(u16vec3(nextI * (sides + 1) + j, nextI * (sides + 1) + nextJ, i * (sides + 1) + nextJ));
        }
    }
}


void Mesh::sphere(float radius, int rings, int sectors)
{
    const float R = 1.0f / (float)(rings - 1);
    const float S = 1.0f / (float)(sectors - 1);

    // Clear previous data
    vertices().clear();
    indices().clear();

    // Generate vertices()
    for (int r = 0; r < rings; r++) {
        for (int s = 0; s < sectors; s++) {
            // Calculate the azimuthal angle (longitude) phi and polar angle (latitude) theta
            float theta = pi<float>() * r * R;       // Range from 0 to pi
            float phi = 2 * pi<float>() * s * S;     // Range from 0 to 2*pi

            // Spherical to Cartesian conversion
            float x = cos(phi) * sin(theta);
            float y = sin(-pi<float>() + theta);  // Latitude from -pi/2 to pi/2
            float z = sin(phi) * sin(theta);

            Vertex vertex;
            vertex.position = vec4(x * radius, y * radius, z * radius, 1.0f);
            vertex.normal = vec4(x, y, z, 0.0f); // Normal is the same as the position for a sphere
            vertices().push_back(vertex);
        }
    }

    // Generate indices
    for (int r = 0; r < rings - 1; r++) {
        for (int s = 0; s < sectors - 1; s++) {
            int curRow = r * sectors;
            int nextRow = (r + 1) * sectors;

            indices().push_back(u16vec3(curRow + s, nextRow + s, nextRow + (s + 1)));
            indices().push_back(u16vec3(curRow + s, nextRow + (s + 1), curRow + (s + 1)));
        }
    }
}

void Mesh::quad()
{
    vertices().emplace_back(Vertex{ .position = glm::vec4(-1, -1, 0, 1), .uv = glm::vec2(0.0f, 0.0f) });
    vertices().emplace_back(Vertex{ .position = glm::vec4(1, -1, 0, 1), .uv = glm::vec2(1.0f, 0.0f) });
    vertices().emplace_back(Vertex{ .position = glm::vec4(1, 1, 0, 1), .uv = glm::vec2(1.0f, 1.0f) });
    vertices().emplace_back(Vertex{ .position = glm::vec4(-1, 1, 0, 1), .uv = glm::vec2(0.0f, 1.0f) });
    
    indices().push_back(u16vec3(0, 1, 2));    
    indices().push_back(u16vec3(2, 3, 0));
}

void Mesh::copy(const Mesh &rhs)
{
    vertexData  = rhs.vertexData;
    indicesData = rhs.indicesData;
}

glm::vec3 Box::center() const
{
    return (min + max) * 0.5f;
}

void Box::expand(const glm::vec3 &point)
{
    min.x = std::min(min.x, point.x);
    min.y = std::min(min.y, point.y);
    min.z = std::min(min.z, point.z);

    max.x = std::max(max.x, point.x);
    max.y = std::max(max.y, point.y);
    max.z = std::max(max.z, point.z);
}
