#pragma once

template<typename ValueType>
class UnitVector3;

template<typename ValueType>
class Vector3
{
public:

    ValueType X;
    ValueType Y;
    ValueType Z;

    static const int Dimension = 3;
    ValueType GetLength() const 
    {
        return sqrt(X * X + Y * Y + Z * Z);
    }
    Vector3 operator+(const Vector3 &value) const
    {
        return Vector3(X + value.X, Y + value.Y, Z + value.Z);
    }

    Vector3 operator+(const UnitVector3<ValueType> &value) const
    {
        return Vector3(X + value.X, Y + value.Y, Z + value.Z);
    }

    Vector3 operator-(const Vector3 &value) const
    {
        return Vector3(X - value.X, Y - value.Y, Z - value.Z);
    }

    Vector3 operator-(const UnitVector3<ValueType> &value) const
    {
        return Vector3(X - value.X, Y - value.Y, Z - value.Z);
    }

    Vector3 operator-() const
    {
        return Vector3(-X, -Y, -Z);
    }

    ValueType operator*(const Vector3 &value) const
    {
        return X * value.X + Y * value.Y + Z * value.Z;
    }

    ValueType operator*(const UnitVector3<ValueType> &value) const
    {
        return X * value.X + Y * value.Y + Z * value.Z;
    }

    Vector3 operator*(const ValueType &value) const
    {
        return Vector3(X * value, Y * value, Z * value);
    }

    Vector3 operator/(const ValueType &value) const
    {
        #if DEBUG
        if (value == ValueType(0.0))
            throw DividedByZeroException();
        #endif
        return Vector3(X / value, Y / value, Z / value);
    }

    Vector3 operator^(const Vector3 &value) const
    {
        return Vector3(
            Y * value.Z - Z * value.Y,
            Z * value.X - X * value.Z,
            X * value.Y - Y * value.X);
    }

    Vector3 operator^(const UnitVector3<ValueType> &value) const
    {
        return Vector3(
            Y * value.Z - Z * value.Y,
            Z * value.X - X * value.Z,
            X * value.Y - Y * value.X);
    }

    Vector3 &operator+=(const Vector3 &value)
    {
        X += value.X;
        Y += value.Y;
        Z += value.Z;
        return *this;
    }

    Vector3 &operator+=(const UnitVector3<ValueType> &value)
    {
        X += value.X;
        Y += value.Y;
        Z += value.Z;
        return *this;
    }

    Vector3 &operator-=(const Vector3 &value)
    {
        X -= value.X;
        Y -= value.Y;
        Z -= value.Z;
        return *this;
    }

    Vector3 &operator-=(const UnitVector3<ValueType> &value)
    {
        X -= value.X;
        Y -= value.Y;
        Z -= value.Z;
        return *this;
    }

    Vector3 &operator*=(const ValueType value)
    {
        X *= value;
        Y *= value;
        Z *= value;
        return *this;
    }

    Vector3 &operator/=(const ValueType value)
    {
        #if DEBUG
        if (value == ValueType(0.0))
            throw DividedByZeroException();
        #endif
        X /= value;
        Y /= value;
        Z /= value;
        return *this;
    }

public:
    Vector3() : X(ValueType()), Y(ValueType()), Z(ValueType()) { }
    Vector3(const UnitVector3<ValueType> &v) : X(v.X), Y(v.Y), Z(v.Z) { }
    Vector3(const ValueType &unique_value) : X(unique_value), Y(unique_value), Z(unique_value) { }
    Vector3(const ValueType &x, const ValueType &y) : X(x), Y(y), Z(ValueType(0.0)) { }
    Vector3(const ValueType &x, const ValueType &y, const ValueType &z) : X(x), Y(y), Z(z) { }
};

// UnitVector3
template<typename ValueType>
class UnitVector3
{
private:

    ValueType X_;
    ValueType Y_;
    ValueType Z_;
public:

    const static UnitVector3 Invalid;

    __declspec(property(get=GetX)) ValueType X;
    ValueType GetX() const
    {
        return X_;
    }

    __declspec(property(get=GetY)) ValueType Y;
    ValueType GetY() const
    {
        return Y_;
    }

    __declspec(property(get=GetZ)) ValueType Z;
    ValueType GetZ() const
    {
        return Z_;
    }
    bool GetIsInvalid() const
    {
        return (X_ == ValueType(0.0)) && (Y_ == ValueType(0.0)) && 
            (Z_ == ValueType(0.0));
    }

  public:
    static UnitVector3 GetZero()
    {
        UnitVector3 retval;
        retval.X_ = ValueType(0.0);
        retval.Y_ = ValueType(0.0);
        retval.Z_ = ValueType(0.0);
        return retval;
    }

    static UnitVector3 GetInvalid()
    {
        UnitVector3 retval;
        retval.X_ = ValueType(0.0);
        retval.Y_ = ValueType(0.0);
        retval.Z_ = ValueType(0.0);
        return retval;
    }

    Vector3<ValueType> operator+(const Vector3<ValueType> &value) const
    {
        return Vector3<ValueType>(X_ + value.X, Y_ + value.Y, Z_ + value.Z);
    }

    Vector3<ValueType> operator+(const UnitVector3 &value) const
    {
        return Vector3<ValueType>(X_ + value.X, Y_ + value.Y, Z_ + value.Z);
    }

    Vector3<ValueType> operator-(const Vector3<ValueType> &value) const
    {
        return Vector3<ValueType>(X_ - value.X, Y_ - value.Y, Z_ - value.Z);
    }

    Vector3<ValueType> operator-(const UnitVector3 &value) const
    {
        return Vector3<ValueType>(X_ - value.X, Y_ - value.Y, Z_ - value.Z);
    }

    UnitVector3 operator-() const
    {
        return UnitVector3(-X_, -Y_, -Z_);
    }

    ValueType operator*(const Vector3<ValueType> &value) const
    {
        return X_ * value.X + Y_ * value.Y + Z_ * value.Z;
    }

    ValueType operator*(const UnitVector3 &value) const
    {
        return X_ * value.X + Y_ * value.Y + Z_ * value.Z;
    }

    Vector3<ValueType> operator*(const ValueType &value) const
    {
        return Vector3<ValueType>(X_ * value, Y_ * value, Z_ * value);
    }

    Vector3<ValueType> operator/(const ValueType &value) const
    {
#if DEBUG
        if (value == ValueType(0.0))
            throw DividedByZeroException();
#endif
        return Vector3<ValueType>(X_ / value, Y_ / value, Z_ / value);
    }

    Vector3<ValueType> operator^(const Vector3<ValueType> &value) const
    {
        return Vector3<ValueType>(
            Y_ * value.Z - Z_ * value.Y,
            Z_ * value.X - X_ * value.Z,
            X_ * value.Y - Y_ * value.X);
    }

    Vector3<ValueType> operator^(const UnitVector3 &value) const
    {
        return Vector3<ValueType>(
            Y_ * value.Z - Z_ * value.Y,
            Z_ * value.X - X_ * value.Z,
            X_ * value.Y - Y_ * value.X);
    }

    const ValueType &operator[](int index) const
    {
#if DEBUG
        if ((index < 0) || (index >= 3))
            throw ArgumentOutOfRangeException("Index out of range.", "index");
#endif
        return ((const ValueType *)this)[index];
    }

      #pragma endregion

public:

    ValueType GetLength() const
    {
        return one(ValueType);
    }

public:

    bool operator==(const Vector3<ValueType> &o) const
    {
        return (X_ == o.X) && (Y_ == o.Y) && (Z_ == o.Z);
    }

    bool operator==(const UnitVector3 &o) const
    {
        return (X_ == o.X) && (Y_ == o.Y) && (Z_ == o.Z);
    }

    bool operator!=(const Vector3<ValueType> &o) const
    {
        return (X_ != o.X) || (Y_ != o.Y) || (Z_ != o.Z);
    }

    bool operator!=(const UnitVector3 &o) const
    {
        return (X_ != o.X) || (Y_ != o.Y) || (Z_ != o.Z);
    }

    UnitVector3 GetUnit() const
    {
        return *this;
    }

    void SetUnit()
    {
        // Already unit, does nothing
    }

    Vector3<ValueType> ToVector3() const
    {
        return Vector3<ValueType>(X_, Y_, Z_);
    }

private:
    // Used only for constructors.
    void InternalSetUnit()
    {
        if (!this->GetIsInvalid())
        {
            ValueType X2 = X_ * X_;
            ValueType Y2 = Y_ * Y_;
            ValueType Z2 = Z_ * Z_;
            ValueType l = std::sqrt(X2 + Y2 + Z2);
            X_ /= l;
            Y_ /= l;
            Z_ /= l;
        }
    }

public:
    UnitVector3() : X_(ValueType(1.0)), Y_(ValueType(0.0)), Z_(ValueType(0.0))
    { }

    // Automatically normalizes.
    UnitVector3(const ValueType &x, const ValueType &y, const ValueType &z)
        : X_(x), Y_(y), Z_(z)
    {
        InternalSetUnit();
    }

    // Automatically normalizes.
    UnitVector3(const ValueType &x, const ValueType &y)
        : X_(x), Y_(y), Z_(ValueType(0.0))
    {
        InternalSetUnit();
    }

    // Automatically normalizes.
    UnitVector3(const ValueType &unique_value)
        : X_(unique_value), Y_(unique_value), Z_(unique_value)
    {
        InternalSetUnit();
    }

};

typedef Vector3<double> vec3;
typedef UnitVector3<double> uvec3;