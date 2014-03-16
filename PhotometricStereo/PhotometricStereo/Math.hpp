#pragma once

#include "stdafx.h"
#include "PhotometricStereoMath.h"
#include "Vector3.h"

#define PI 3.1415926535897932384626433832795

template <typename T>
class VectorPtr
{
    typedef ::std::shared_ptr<vector<T>> VectorPtrInternal;
private:
    VectorPtrInternal Ptr_;
public:
    template<typename T0>
    VectorPtr(T0 p) : Ptr_(make_shared<VectorPtrInternal::element_type>(p)) { }

    VectorPtr(::std::nullptr_t p) : Ptr_(p) { }
    template<typename T0, typename T1>
    VectorPtr(T0 p, T1 p1) : Ptr_(make_shared<VectorPtrInternal::element_type>(p, p1)) { }
    VectorPtr(const VectorPtr &p) : Ptr_(p.Ptr_) { }
    VectorPtr(const VectorPtrInternal &p = nullptr) : Ptr_(p) { }
    typename VectorPtrInternal::element_type::value_type operator [] (int index) const { return Ptr_->operator[] (index); }
    typename VectorPtrInternal::element_type::value_type &operator [] (int index) { return Ptr_->operator[] (index); }
    int __declspec(property(get=GetDimension)) Dimension;
    int GetDimension() const { return Ptr_->size(); }
    typename VectorPtrInternal::element_type *operator->() { return Ptr_.get(); }
    bool operator== (const VectorPtr &other) const  { return Ptr_ == other.Ptr_; }
    
    bool __declspec(property(get=GetIsNull)) IsNull;
    bool GetIsNull() const { return Ptr_ == nullptr; }
    typedef vector<T> VectorType;
    typedef T ValueType;
};

template<typename T>
void Write(ostream &out, const VectorPtr<T> &p)
{
    Write(out, p.Dimension);
    for (int i = 0; i < p.Dimension; ++i)
        Write(out, p[i]);
}
template<typename T>
void Read(istream &Y,  VectorPtr<T> &p)
{
    int dimension;
    Read(Y, dimension);
    p = VectorPtr<T>(dimension);
    for (int i = 0; i < dimension; ++i)
        Read(Y, p[i]);
}
VectorPtr<double> LinearLeastSquare(double *A, double *b, int M, int N);
