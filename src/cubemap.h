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

};