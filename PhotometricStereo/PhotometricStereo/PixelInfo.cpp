#include "stdafx.h"

#include "ImageModel.h"
#include "PixelInfo.h"

int PixelInfo::GetDimension() const
{
    return ImageModel_->NormalCube->C;
}

bool PixelInfo::GetIsInvalid() const
{
    return ImageModel_ == nullptr || Index_.X < 0 || Index_.Y < 0 || Index_.Y >= ImageModel_->NormalCube->N || Index_.X >= ImageModel_->NormalCube->M;
}

bool PixelInfo::DarkPixel() const
{
    if (Dark_ != 0)
        return Dark_ == 2;
    bool dark = true;
    if (IsInvalid)
        dark = true;
    else
    {
        int dimension = Dimension;
        auto *data = GetData<NormalType>(NormalCube);
        for (int i = 0; i < dimension; i++)
        {
            if (data[i] > 20)
                dark = false;
        }
    }
    Dark_ = dark ? 2 : 1;
    return dark;
}

std::string PixelInfo::LightningVectorStr() const
{
    const NormalType *data = GetData<NormalType>(NormalCube);
    ostringstream oss("(");
    for (int i = 0; i < Dimension - 1; ++i)
    {
        oss << data[i] << ", ";
    }
    oss << data[Dimension - 1] << ")";
    return oss.str();
}

bool PixelInfo::BoundaryPixel() const
{
    if (Boundary_ != 0)
        return Boundary_ == 2;
    if (!DarkPixel() && ImageModel_ != nullptr)
    {
        PixelInfo pi = ImageModel_->CreatePixelInfo(Index_.X - 1, Index_.Y);
        if (!pi.IsInvalid && pi.DarkPixel())
        {
            Boundary_ = 2;
            return true;
        }
        pi = ImageModel_->CreatePixelInfo(Index_.X + 1, Index_.Y);
        if (!pi.IsInvalid && pi.DarkPixel())
        {
            Boundary_ = 2;
            return true;
        }
        pi = ImageModel_->CreatePixelInfo(Index_.X, Index_.Y - 1);
        if (!pi.IsInvalid && pi.DarkPixel())
        {
            Boundary_ = 2;
            return true;
        }
        pi = ImageModel_->CreatePixelInfo(Index_.X, Index_.Y + 1);
        if (!pi.IsInvalid && pi.DarkPixel())
        {
            Boundary_ = 2;
            return true;
        }
    }
    Boundary_ = 1;
    return false;
}

double PixelInfo::GetManhatonDistance( const NormalType *vector, bool self_reference, const double *gray_scal_weight )
{
    if (vector == nullptr)
        return DBL_MAX;
    double sum = 0;
    bool ok = false;
    int dimension = Dimension;
    NormalType *data = GetData<NormalType>(NormalCube); 
    for (int i = 0; i < dimension; i++)
    {
        if (vector[i] < 70)
            continue;
        double factor = self_reference ? gray_scal_weight[(int)(data[i])] : gray_scal_weight[(int)(vector[i])];
        ok = true;
        sum += factor * ::std::abs(data[i] - vector[i]);
    }
    if (ok)
        return sum;
    return DBL_MAX;
}

double PixelInfo::GetEuclideanDistance( const NormalType *vector, bool self_reference, const double *gray_scal_weight )
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
        sum += factor * (data[i] - vector[i]) * (data[i] - vector[i]);
    }
    if (ok)
        return sum;
    return DBL_MAX;
}

// NOTE: these two persistent methods for PixelInfo only read and write the normal and position info to/from file.
// The ImageModel_ info will be lost!
void Write(ostream &os, const PixelInfo t)
{
    Write(os, t.Index.X);
    Write(os, t.Index.Y);
    // here, call DarkPixel to calc Dark_
    t.DarkPixel();
    Write(os, t.Dark_);
    // here, call BoundaryPixel to calc Boundary_
    t.BoundaryPixel();
    Write(os, t.Boundary_);
    Write(os, t.Normal.X);
    Write(os, t.Normal.Y);
    Write(os, t.Normal.Z);
    Write(os, t.ActualNormal.X);
    Write(os, t.ActualNormal.Y);
    Write(os, t.ActualNormal.Z);
    Write(os, t.Position.X);
    Write(os, t.Position.Y);
    Write(os, t.Position.Z);
}

void Read(istream &is, PixelInfo &t)
{
    int x, y;
    Read(is, x);
    Read(is, y);
    t.Index.X = x;
    t.Index.Y = y;
    Read(is, t.Dark_);
    Read(is, t.Boundary_);
    double X, Y, Z;
    Read(is, X);
    Read(is, Y);
    Read(is, Z);
    t.Normal = uvec3(X, Y, Z);
    Read(is, X);
    Read(is, Y);
    Read(is, Z);
    t.ActualNormal = uvec3(X, Y, Z);
    Read(is, X);
    Read(is, Y);
    Read(is, Z);
    t.Position = vec3(X, Y, Z);
}

PixelInfo PixelInfo::InvalidPixel;

