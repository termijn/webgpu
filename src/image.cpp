#include "image.h"
#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <random>
#include <cmath>
#include <iostream>

using namespace glm;


Image::Image()
    : pixels(std::make_shared<std::vector<uint8_t>>())
{

}

Image::Image(size_t size)
    : pixels(std::make_shared<std::vector<uint8_t>>(size))
{

}

Image::Image(const std::vector<uint8_t>& pixels)
    : pixels(std::make_shared<std::vector<uint8_t>>(pixels))
{
}

const Image Image::toRGBA() const
{
    Image result(width * height * 4);
    result.width = width;
    result.height = height;
    result.bytesPerPixel = 4;
    result.type = Image::Type::RGBA;
    for (size_t i = 0; i < width * height; ++i)
    {
        result.pixels->at(i * 4 + 0) = pixels->at(i * bytesPerPixel + 0); // Red
        result.pixels->at(i * 4 + 1) = pixels->at(i * bytesPerPixel + 1); // Green
        result.pixels->at(i * 4 + 2) = pixels->at(i * bytesPerPixel + 2); // Blue
        result.pixels->at(i * 4 + 3) = 255;                   // Alpha (set to opaque)
    }
    return result;
}

Image::Image(const Image& rhs)
    : pixels(std::make_shared<std::vector<uint8_t>>())
{
    copy(rhs);
}

Image::~Image() = default;

Image& Image::operator=(const Image& rhs)
{
    copy(rhs);
    return *this;
}

int Image::sizeInBytes() const
{
    return width * height * bytesPerPixel;
}

void Image::copy(const Image &rhs)
{
    if (this == &rhs) return;

    width           = rhs.width;
    height          = rhs.height;
    bytesPerPixel   = rhs.bytesPerPixel;
    type            = rhs.type;
    pixels          = rhs.pixels;
}

vec2 randomPointInAnnulus(const vec2 &center, float minDistance, float maxDistance)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> angleDist(0, 2 * glm::pi<double>());
    std::uniform_real_distribution<> radiusDist(minDistance, maxDistance);

    double angle     = angleDist(gen);
    double radius    = radiusDist(gen);

    return vec2(center.x + radius * std::cos(angle), center.y + radius * std::sin(angle));
}

std::vector<vec2> generatePoissonDiscPoints(float width, float height, float minDistance, int newPointAttempts)
{
    double cellSize = minDistance / std::sqrt(2); // The grid cell size
    int gridWidth = static_cast<int>(std::ceil(width / cellSize));
    int gridHeight = static_cast<int>(std::ceil(height / cellSize));

    // Grid to hold points
    std::vector<std::vector<vec2 *>> grid(gridWidth, std::vector<vec2 *>(gridHeight, nullptr));

    // Active list of points
    std::vector<vec2> activeList;
    std::vector<vec2> points; // Final set of points

    // Randomly select the first point
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> widthDist(0, width);
    std::uniform_real_distribution<> heightDist(0, height);

    vec2 initialPoint(widthDist(gen), heightDist(gen));
    points.push_back(initialPoint);
    activeList.push_back(initialPoint);

    int gridX = static_cast<int>(initialPoint.x / cellSize);
    int gridY = static_cast<int>(initialPoint.y / cellSize);
    grid[gridX][gridY] = &points.back();

    while (!activeList.empty())
    {
        int randomIndex = std::uniform_int_distribution<>(0, int(activeList.size()) - 1)(gen);
        vec2 point = activeList[randomIndex];
        bool found = false;

        for (int i = 0; i < newPointAttempts; ++i)
        {
            vec2 newPoint = randomPointInAnnulus(point, minDistance, 2 * minDistance);

            if (newPoint.x >= 0 && newPoint.x < width && newPoint.y >= 0 && newPoint.y < height)
            {
                int newGridX = static_cast<int>(newPoint.x / cellSize);
                int newGridY = static_cast<int>(newPoint.y / cellSize);
                bool valid = true;

                // Check neighboring cells
                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        int neighborX = newGridX + dx;
                        int neighborY = newGridY + dy;
                        if (neighborX >= 0 && neighborX < gridWidth && neighborY >= 0 && neighborY < gridHeight)
                        {
                            vec2 *neighbor = grid[neighborX][neighborY];
                            if (neighbor && (newPoint - *neighbor).length() < minDistance)
                            {
                                valid = false;
                            }
                        }
                    }
                }

                if (valid)
                {
                    points.push_back(newPoint);
                    activeList.push_back(newPoint);
                    grid[newGridX][newGridY] = &points.back();
                    found = true;
                    break;
                }
            }
        }

        if (!found)
        {
            activeList.erase(activeList.begin() + randomIndex);
        }
    }

    return points;
}

const uint8_t *Image::getPixels() const
{
    return pixels->data();
}

const size_t Image::bytes() const
{
    return pixels->size();
}

void Image::setPixel(int x, int y, uint8_t value)
{
    assert(bytesPerPixel == 1);
    (*pixels)[width * y + x] = value;
}

void Image::setPixel(int x, int y, uint8_t valueR, uint8_t valueG)
{
    assert(bytesPerPixel >= 2);

    uint8_t* posR = (uint8_t*) &((*pixels)[(width * y + x) * bytesPerPixel]); 
    uint8_t* posG = posR + 1; 

    *posR = valueR;
    *posG = valueG;
}

void Image::setPixel(int x, int y, uint16_t value)
{
    assert(bytesPerPixel == 2);
    uint16_t* pos = (uint16_t*) &((*pixels)[width * y + x * bytesPerPixel]); 
    *pos = value;
}

void Image::setPixel(int x, int y, uint32_t value)
{
    assert(bytesPerPixel == 4);
    uint32_t* pos = (uint32_t*) &((*pixels)[width * y + x * bytesPerPixel]); 
    *pos = value;
}

int Image::makePoissonDisc(int width_, int height_, int minDistance)
{
    std::vector<vec2> points = generatePoissonDiscPoints(float(width_), float(width_), float(minDistance), 30);

    width           = int(points.size());
    height          = 1;
    bytesPerPixel   = 2;
    type            = Type::RG8;
    pixels->resize(sizeInBytes(), 0);

    int xi = 0;
    for (const vec2& point : points)
    {
        int x = static_cast<int>(floor(point.x));
        int y = static_cast<int>(floor(point.y));

        if (x >= 0 && x < width_ && y >= 0 && y < height_) 
        {
            vec2 size           = vec2(width_, height_);
            vec2 coordinate     = vec2(point.x, point.y) / size;

            uint8_t r = static_cast<uint8_t>(coordinate.x * 255.0f);
            uint8_t g = static_cast<uint8_t>(coordinate.y * 255.0f);
            setPixel(xi, 0, r, g);
        }
        xi++;
    }
    return int(points.size());
}
