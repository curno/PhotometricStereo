#pragma once

#include "PixelInfo.h"
#include "PixelInfoSet.h"
#include "ImageDataCube.h"

class ImageModel
{
public:
    shared_ptr<ImageDataCube<ImageDataCubeType<NormalCube>::Type>> NormalCube;
    shared_ptr<ImageDataCube<ImageDataCubeType<ShadowCube>::Type>> ShadowCube;
    shared_ptr<ImageDataCube<ImageDataCubeType<DifferenceCube>::Type>> DifferenceCube;

    PixelInfoSet BallPixels;
    PixelInfoSet ObjectPixels;
    PixelInfoSet TargetObjectPixels;

    shared_ptr<vector<uvec3>> LightningDirections;

    shared_ptr<ImageDataCubeBase> GetImageDataCube(CubeType type) const
    {
        if (type == ::NormalCube)
            return this->NormalCube;
        else if (type == ::ShadowCube)
            return this->ShadowCube;
        else if (type == ::DifferenceCube)
            return this->DifferenceCube;
        else 
            return nullptr;
    }

    PixelInfo CreatePixelInfo(int x, int y)
    {
        PixelInfo retval;
        retval.Index.X = x;
        retval.Index.Y = y;
        retval.SetImageData(this);
        return retval;
    }

};