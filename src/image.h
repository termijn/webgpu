#pragma once

#include <vector>
#include <memory>

class Image
{
public:
    Image();
    Image(size_t size);
    Image(const std::vector<uint8_t>& pixels);
    Image(const Image& rhs);

    virtual ~Image();

    const Image toRGBA() const;

    Image& operator=(const Image& rhs);

    enum class Type {
        RGBA16,
        RGBA,
        RGB,
        RG8,
        R8
    };

    int width;
    int height;
    
    int bytesPerPixel       = 4;
    Type type               = Type::RGBA;

    const uint8_t* getPixels() const;
    const size_t bytes() const;

    void setPixel(int x, int y, uint8_t value);
    void setPixel(int x, int y, uint8_t valueR, uint8_t valueG);
    void setPixel(int x, int y, uint16_t value);
    void setPixel(int x, int y, uint32_t value);

    int makePoissonDisc(int width, int height, int minDistance);
    int sizeInBytes() const;

    std::shared_ptr<std::vector<uint8_t>> pixels;

private:
    void copy(const Image& rhs);
};
