#include "stdafx.h"

#include "PixelInfoSet.h"
#include "Utils.h"

void Write(ostream &os, const PixelInfoSet t)
{
    int size = t.Pixels_->size();
    Write(os, size);
    for (auto i = t.Pixels_->begin(); i != t.Pixels_->end(); ++i)
        Write(os, *i);
}

void Read(istream &is, PixelInfoSet &t)
{
    int count;
    Read<int>(is, count);
    t.Pixels_->reserve(count);
    for (int i = 0; i < count; ++i)
    {
        PixelInfo p;
        Read(is, p);
        t.Pixels_->push_back(p);
    }
}