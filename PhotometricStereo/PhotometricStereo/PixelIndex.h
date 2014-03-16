#pragma once

class PixelIndex
{
private:
    int X_;
    int Y_;

public:
    PixelIndex(int x, int y) : X_(x), Y_(y) { }

    PixelIndex() : X_(-1), Y_(-1) { }

    int __declspec(property(get=GetX, put=SetX)) X;
    int GetX() const { return X_; }
    void SetX(int x) { X_ = x; }

    int __declspec(property(get=GetY, put=SetY)) Y;
    int GetY() const { return Y_; }
    void SetY(int y) { Y_ = y; }

    bool operator==(const PixelIndex &that) const { return this->X_ == that.X_ && this->Y_ == that.Y_; }

    operator QPoint()
    {
        return QPoint(X_, Y_);
    }
};