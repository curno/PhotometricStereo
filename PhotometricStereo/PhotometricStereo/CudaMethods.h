#pragma once
#include "cuda_runtime.h"

cudaError_t addWithCuda(int *c, const int *a, const int *b, unsigned int size);
cudaError_t CudaGetNearestPixelIndex(double *ball_pixel, double *object, double *shadow, 
                                 double *weight, int *result, int ball_pixel_count, int h, int w, int dimension,
                                 int dark_threoshold, int shadow_threoshold, bool manhaton);