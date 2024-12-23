#include "uniformsdata.h"

bool FrameData::operator==(const FrameData& other) const
{
    return 
        view == other.view &&
        projection == other.projection &&
        shadowViewProjection == other.shadowViewProjection &&
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

bool FrameDataShadow::operator==(const FrameDataShadow& other) const
{
    return
        view == other.view &&
        projection == other.projection;
}

bool FrameDataShadow::operator!=(const FrameDataShadow& other) const
{
    return !(*this == other);
}

bool ModelDataShadow::operator==(const ModelDataShadow& other) const
{
    return
        model == other.model;
}

bool ModelDataShadow::operator!=(const ModelDataShadow& other) const
{
    return !(*this == other);
}
