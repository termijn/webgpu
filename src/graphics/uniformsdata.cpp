#include "uniformsdata.h"

bool FrameData::operator==(const FrameData& other) const
{
    return 
        view == other.view &&
        projection == other.projection &&
        viewPositionWorld == other.viewPositionWorld &&
        lightPositionWorld == other.lightPositionWorld;
}

bool FrameData::operator!=(const FrameData& other) const
{
    return !(*this == other);
}

bool ModelData::operator==(const ModelData& other) const
{
    return 
        model == other.model &&
        modelInverseTranspose == other.modelInverseTranspose;
}

bool ModelData::operator!=(const ModelData& other) const
{
    return !(*this == other);
}

