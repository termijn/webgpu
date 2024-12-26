#pragma once

#include "image.h"

class Cubemap
{
public:
    Image positiveX;
    Image negativeX;
    Image positiveY;
    Image negativeY;
    Image positiveZ;
    Image negativeZ;

    Cubemap() = default;

    Cubemap(const Image& positiveX, const Image& negativeX, const Image& positiveY, const Image& negativeY, const Image& positiveZ, const Image& negativeZ)
        : positiveX(positiveX)
        , negativeX(negativeX)
        , positiveY(positiveY)
        , negativeY(negativeY)
        , positiveZ(positiveZ)
        , negativeZ(negativeZ)
    {
    }

    const Image& faces(int i) const
    {
        switch (i)
        {
            case 0: return positiveX;
            case 1: return negativeX;
            case 2: return positiveY;
            case 3: return negativeY;
            case 4: return positiveZ;
            case 5: return negativeZ;
            default: throw std::runtime_error("Invalid face index");
        }
    }
};