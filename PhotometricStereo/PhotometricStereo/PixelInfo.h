#pragma once

#include "Math.hpp"
#include "Utils.h"
#include "PixelIndex.h"
class ImageModel;
class ImageDataCubeIndex;
class PixelInfoSet;

enum CubeType { NormalCube, ShadowCube, DifferenceCube };

template<int CubeType>
struct ImageDataCubeType;

template<>
struct ImageDataCubeType<NormalCube> { typedef int Type; };

template<>
struct ImageDataCubeType<ShadowCube> { typedef double Type; };

template<>
struct ImageDataCubeType<DifferenceCube> { typedef double Type; };

typedef ImageDataCubeType<::NormalCube>::Type NormalType;
typedef ImageDataCubeType<::ShadowCube>::Type ShadowType;
typedef ImageDataCubeType<::DifferenceCube>::Type DifferenceType;

class PixelInfo
{
    friend class PixelInfoSet;
    friend class ImageModel;
    friend class PSModel;
    friend void Write(ostream &os, const PixelInfo t);
    friend void Read(istream &is, PixelInfo &t);

private:
    PixelIndex Index_;
    ImageModel *ImageModel_;
    uvec3 Normal_;  // This is the normal we estimate in the PS process.
    uvec3 ActualNormal_; // This is the normal we calculate using the depth values we estimate.
    vec3 Position_;
    mutable byte Dark_;
    mutable byte Boundary_;
public:
    PixelInfo(): ImageModel_(nullptr), Index_(-1, -1), Dark_(0), Boundary_(0) { } 

    PixelIndex readonly(Index);
    PixelIndex &GetIndex() { return Index_; }
    const PixelIndex &GetIndex() const { return Index_; }

    uvec3 readwrite(Normal);
    uvec3 &GetNormal() { return Normal_; }
    const uvec3 &GetNormal() const { return Normal_; }
    void SetNormal(uvec3 v) { Normal_ = v; }

    uvec3 readwrite(ActualNormal);
    uvec3 &GetActualNormal() { return ActualNormal_; }
    const uvec3 &GetActualNormal() const { return ActualNormal_; }
    void SetActualNormal(uvec3 v) { ActualNormal_ = v; }

    vec3 readwrite(Position);
    vec3 &GetPosition() { return Position_; }
    const vec3 &GetPosition() const { return Position_; }
    void SetPosition(vec3 v) { Position_ = v; }

    void SetImageData(ImageModel *model) { ImageModel_ = model; }

    int readonly(Dimension);
    int GetDimension() const;

    bool readonly(IsInvalid);
    bool GetIsInvalid() const;

    template<typename DataType>
    DataType *GetData(CubeType cube_index);

    template<typename DataType>
    const DataType *GetData(CubeType cube_index) const;

    bool DarkPixel() const;
    bool BoundaryPixel() const;
    void SetDarkPixel(bool dark) { Dark_ = (dark ? 2 : 1); }
    string LightningVectorStr() const;

    double GetManhatonDistance(const NormalType *vector, bool self_reference, const double *gray_scal_weight)
    {
        if (vector == nullptr)
            return DBL_MAX;
        double sum = 0;
        bool ok = false;
        int dimension = Dimension;
        NormalType *data = GetData<NormalType>(NormalCube); 
        for (int i = 0; i < dimension; i++)
        {
            double factor = self_reference ? gray_scal_weight[(int)(data[i])] : gray_scal_weight[(int)(vector[i])];
            ok = true;
            sum += factor * ::std::abs(data[i] - vector[i]);
        }
        if (ok)
            return sum;
        return DBL_MAX;
    }

    double GetManhationDistanceConsideringShadow(const NormalType *vector, bool self_reference, const ShadowType *shadow, double threhold, 
        const double *gray_scal_weight)
    {
        if (vector == nullptr)
            return DBL_MAX;
        double sum = 0;
        bool ok = false;
        int dimension = Dimension;
        for (int i = 0; i < dimension; i++)
        {
            if (abs(shadow[i]) > threhold)
                continue;
            const NormalType *data = GetData<NormalType>(NormalCube); 
            double factor = self_reference ? gray_scal_weight[(int)(data[i])] : gray_scal_weight[(int)(vector[i])];
            ok = true;
            sum += factor * ::std::abs(data[i] - vector[i]);
        }
        if (ok)
            return sum;
        return DBL_MAX;
    }

    double GetAngleDistance(const NormalType *vector)
    {
        if (vector == nullptr)
            return DBL_MAX;
        double sum = 0;
        double l1 = 0; 
        double l2 = 0;
        int dimension = Dimension;
        NormalType *data = GetData<NormalType>(NormalCube); 
        for (int i = 0; i < dimension; i++)
        {
            sum += vector[i] * data[i];
            l1 += vector[i] * vector[i];
            l2 += data[i] * data[i];
        }
        double v = max(0.0, min(1.0, sum / sqrt(l1) / sqrt(l2)));
        return acos(v);

    }

    double GetAngleDistanceConsideringShadow(const NormalType *vector, const ShadowType *shadow, double threhold)
    {
        if (vector == nullptr)
            return DBL_MAX;
        double sum = 0;
        double l1 = 0;
        double l2 = 0;
        bool ok = false;
        int dimension = Dimension;
        for (int i = 0; i < dimension; i++)
        {
            if (abs(shadow[i]) > threhold)
                continue;
            const NormalType *data = GetData<NormalType>(NormalCube); 
            ok = true;
            sum += vector[i] * data[i];
            l1 += vector[i] * vector[i];
            l2 += data[i] * data[i];
        }
        if (ok)
        {
            double v = max(0.0, min(1.0, sum / sqrt(l1) / sqrt(l2)));
            return acos(v);
        }
        return DBL_MAX;
    }

    static PixelInfo InvalidPixel;
};

void Write(ostream &os, const PixelInfo t);
void Read(istream &os, PixelInfo &t);

