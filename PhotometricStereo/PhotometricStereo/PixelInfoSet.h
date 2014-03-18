#pragma once

#include "stdafx.h"
#include "PixelInfo.h"

class PixelInfoSet
{
    friend void Write(ostream &os, const PixelInfoSet t);
    friend void Read(istream &is, PixelInfoSet &t);
private:
    std::shared_ptr<vector<PixelInfo>> Pixels_;

public:
    PixelInfoSet(int capacity = 0) : Pixels_(std::make_shared<vector<PixelInfo>>()) 
    {
        if (capacity != 0)
            Pixels_->reserve(capacity);
    }
    
    PixelInfo &operator[] (int index){ return Pixels_->operator [](index); }
    const PixelInfo &operator[] (int index) const { return Pixels_->operator [](index); }
    vector<PixelInfo> *operator-> () { return Pixels_.get(); }
    vector<PixelInfo> &operator*() { return *operator->(); }
    void SetData(vector<PixelInfo> *ptr) { Pixels_ = std::shared_ptr<vector<PixelInfo>>(ptr); }

    
};

void Write(ostream &os, const PixelInfoSet t);
void Read(istream &is, PixelInfoSet &t);
