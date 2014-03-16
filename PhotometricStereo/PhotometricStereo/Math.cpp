#include "stdafx.h"
#include "Math.hpp"

#ifndef NO_INIT 
static class __MathInit
{
public:
    __MathInit() { PhotometricStereoMathInitialize(); }
    ~__MathInit() { PhotometricStereoMathTerminate(); }
} __MathInit__;

#endif

VectorPtr<double> LinearLeastSquare(double *A, double *b, int M, int N)
{

    mxArray *mat_A = mxCreateDoubleMatrix(M, N, mxREAL);
    mxArray *vec_b = mxCreateDoubleMatrix(M, 1, mxREAL);

    mxArray *input[2] = {mat_A, vec_b};
    mxArray *output[1] = { nullptr };

    double *mat_A_data = static_cast<double *>(mxGetPr(mat_A));
    memcpy(mat_A_data, A, M * N * sizeof(double));
    double *vec_b_data = static_cast<double *>(mxGetPr(vec_b));
    memcpy(vec_b_data, b, M * 1 * sizeof(double));

    mlxLinearleastsquare(1, output, 2, input);

    VectorPtr<double> retval(N);
    retval->assign(static_cast<VectorPtr<double>::ValueType *>(mxGetData(output[0])), static_cast<VectorPtr<double>::ValueType *>(mxGetData(output[0])) + N);

    return retval;
}



