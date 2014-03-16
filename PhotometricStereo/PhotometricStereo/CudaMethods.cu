#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>

__global__ void GetNearestPixelIndexKernel_Manhaton(double *ball_pixel, double *object, double *shadow, 
                                           double *weight, int *result, int ball_pixel_count, int dimension,
                                           int dark_threoshold, int shadow_threoshold, int start, int end)
{
    int index = start + blockDim.x * blockIdx.x + threadIdx.x;
    if (index >= end)
        return;
    object += (index * dimension);
    shadow += (index * dimension);

    bool dark = true;
    for (int i = 0; i < dimension; ++i)
        if (object[i] > dark_threoshold)
        {
            dark = false;
            break;
        }
    if (dark)
    {
        result[index] = -1;
        return;
    }

    double minimun_dis = 1.0e300;
    double minimum_idx = -1;
    for (int i = 0; i < ball_pixel_count; ++i)
    {
        double dis = 0;
        bool shadow_flag = true;
        for (int j = 0; j < dimension; ++j)
        {
            if ((shadow[j] > 0 ? shadow[j] : -shadow[j]) > shadow_threoshold)
                continue;
            shadow_flag = false;
            dis += ((object[j] > ball_pixel[j]) ? (object[j] - ball_pixel[j]) : (ball_pixel[j] - object[j])) * weight[static_cast<int>(object[j])];
        }
        if (!shadow_flag)
        {
            if (minimun_dis > dis)
            {
                minimun_dis = dis;
                minimum_idx = i;
            }
        }
        ball_pixel += dimension;
    }
    result[index] = minimum_idx;
    return;
}

__global__ void GetNearestPixelIndexKernel_Angle(double *ball_pixel, double *object, double *shadow, 
                                           int *result, int ball_pixel_count, int dimension,
                                           int dark_threoshold, int shadow_threoshold, int start, int end)
{
    int index = start + blockDim.x * blockIdx.x + threadIdx.x;
    if (index >= end)
        return;
    object += (index * dimension);
    shadow += (index * dimension);

    bool dark = true;
    for (int i = 0; i < dimension; ++i)
        if (object[i] > dark_threoshold)
        {
            dark = false;
            break;
        }
    if (dark)
    {
        result[index] = -1;
        return;
    }

    double maximum_dis = -1.0e300;
    double maximum_idx = -1;
    for (int i = 0; i < ball_pixel_count; ++i)
    {
        double dis = 0;
        double l1 = 0;
        bool shadow_flag = true;
        for (int j = 0; j < dimension; ++j)
        {
            if ((shadow[j] > 0 ? shadow[j] : -shadow[j]) > shadow_threoshold)
                continue;
            shadow_flag = false;
            dis += object[j] * ball_pixel[j];
            l1 += ball_pixel[j] * ball_pixel[j];
        }
        if (!shadow_flag)
        {
            dis = dis / sqrt(l1);
            if (maximum_dis < dis)
            {
                maximum_dis = dis;
                maximum_idx = i;
            }
        }
        ball_pixel += dimension;
    }
    result[index] = maximum_idx;
    return;
}

__global__ void GetNearestPixelIndexKernel_Manhaton_NoShadow(double *ball_pixel, double *object, 
                                           double *weight, int *result, int ball_pixel_count, int dimension,
                                           int dark_threoshold, int start, int end)
{
    int index = start + blockDim.x * blockIdx.x + threadIdx.x;
    if (index >= end)
        return;
    object += (index * dimension);

    bool dark = true;
    for (int i = 0; i < dimension; ++i)
        if (object[i] > dark_threoshold)
        {
            dark = false;
            break;
        }
    if (dark)
    {
        result[index] = -1;
        return;
    }

    double minimun_dis = 1.0e300;
    double minimum_idx = -1;
    for (int i = 0; i < ball_pixel_count; ++i)
    {
        double dis = 0;
        for (int j = 0; j < dimension; ++j)
            dis += ((object[j] > ball_pixel[j]) ? (object[j] - ball_pixel[j]) : (ball_pixel[j] - object[j])) * weight[static_cast<int>(object[j])];
        if (minimun_dis > dis)
        {
            minimun_dis = dis;
            minimum_idx = i;
        }
        ball_pixel += dimension;
    }
    result[index] = minimum_idx;
    return;
}

__global__ void GetNearestPixelIndexKernel_Angle_NoShadow(double *ball_pixel, double *object, 
                                           int *result, int ball_pixel_count, int dimension,
                                           int dark_threoshold, int start, int end)
{
    int index = start + blockDim.x * blockIdx.x + threadIdx.x;
    if (index >= end)
        return;
    object += (index * dimension);

    bool dark = true;
    for (int i = 0; i < dimension; ++i)
        if (object[i] > dark_threoshold)
        {
            dark = false;
            break;
        }
    if (dark)
    {
        result[index] = -1;
        return;
    }

    double maximum_dis = -1.0e300;
    double maximum_idx = -1;
    for (int i = 0; i < ball_pixel_count; ++i)
    {
        double dis = 0;
        double l1 = 0;
        for (int j = 0; j < dimension; ++j)
        {
            dis += object[j] * ball_pixel[j];
            l1 += ball_pixel[j] * ball_pixel[j];
        }
        dis = dis / sqrt(l1);
        if (maximum_dis < dis)
        {
            maximum_dis = dis;
            maximum_idx = i;
        }
        ball_pixel += dimension;
    }
    result[index] = maximum_idx;
    return;
}

cudaError_t CudaGetNearestPixelIndex(double *ball_pixel, double *object, double *shadow, 
                                 double *weight, int *result, int ball_pixel_count, int h, int w, int dimension,
                                 int dark_threoshold, int shadow_threoshold, bool manhaton)
{
    typedef double LightningType;
    cudaError_t cuda_status;
    int object_size = w * h;
    int ball_pixel_lighning_vector_size = dimension * ball_pixel_count;
    int object_pixel_lightning_vector_size = dimension * object_size;
    int object_pixel_shadow_vector_size = object_pixel_lightning_vector_size;
    const int gray_scale_weight_size = 256;

    LightningType *ptr_ball_pixel = nullptr, 
           *ptr_object_pixel = nullptr, 
           *ptr_object_shadow = nullptr;

    int *ptr_result = nullptr;
    double *ptr_gray_scale_weight = nullptr;

    int per_batch_count = 399999999 / ball_pixel_count / dimension + 1;
    int batch_count = ((object_size % per_batch_count == 0) ? 0 : 1) + object_size / per_batch_count;

    int threads_num = 256;
    int blocks_num = per_batch_count / threads_num + (per_batch_count % threads_num == 0 ? 0 : 1);

    int i;
    // Choose which GPU to run on, change this on a multi-GPU system.
    cuda_status = cudaSetDevice(0);
    if (cuda_status != cudaSuccess) {
        goto Error;
    }

    cuda_status = cudaMalloc((void**)&ptr_ball_pixel, ball_pixel_lighning_vector_size * sizeof(LightningType));
    if (cuda_status != cudaSuccess)
        goto Error;
    cuda_status = cudaMemcpy(ptr_ball_pixel, ball_pixel, ball_pixel_lighning_vector_size * sizeof(LightningType), cudaMemcpyHostToDevice);
    if (cuda_status != cudaSuccess)
        goto Error;   

    cuda_status = cudaMalloc((void**)&ptr_object_pixel, object_pixel_lightning_vector_size * sizeof(LightningType));
    if (cuda_status != cudaSuccess)
        goto Error;
    cuda_status = cudaMemcpy(ptr_object_pixel, object, object_pixel_lightning_vector_size * sizeof(LightningType), cudaMemcpyHostToDevice);
    if (cuda_status != cudaSuccess)
        goto Error;   

    if (shadow != nullptr)
    {
        cuda_status = cudaMalloc((void**)&ptr_object_shadow, object_pixel_shadow_vector_size * sizeof(LightningType));
        if (cuda_status != cudaSuccess)
            goto Error;
        cuda_status = cudaMemcpy(ptr_object_shadow, shadow, object_pixel_shadow_vector_size * sizeof(LightningType), cudaMemcpyHostToDevice);
        if (cuda_status != cudaSuccess)
            goto Error;   
    }

    if (weight != nullptr)
    {
        cuda_status = cudaMalloc((void**)&ptr_gray_scale_weight, gray_scale_weight_size * sizeof(double));
        if (cuda_status != cudaSuccess)
            goto Error;
        cuda_status = cudaMemcpy(ptr_gray_scale_weight, weight, gray_scale_weight_size * sizeof(double), cudaMemcpyHostToDevice);
        if (cuda_status != cudaSuccess)
            goto Error;   
    }

    cuda_status = cudaMalloc((void**)&ptr_result, object_size * sizeof(int));
    if (cuda_status != cudaSuccess)
        goto Error;

    /*cuda_status = cudaMemcpy(ptr_result, result, object_size * sizeof(int), cudaMemcpyHostToDevice);
    if (cuda_status != cudaSuccess)
        goto Error;   */

    for (i = 0; i < batch_count; ++i)
    {
        int start = i * per_batch_count;
        int end = (i + 1) * per_batch_count > object_size ? object_size : (i + 1) * per_batch_count;
        if (manhaton && shadow != nullptr)
            GetNearestPixelIndexKernel_Manhaton<<<blocks_num, threads_num>>>(ptr_ball_pixel,
                                                        ptr_object_pixel,
                                                        ptr_object_shadow,
                                                        ptr_gray_scale_weight,
                                                        ptr_result, 
                                                        ball_pixel_count, 
                                                        dimension,
                                                        dark_threoshold,
                                                        shadow_threoshold,
                                                        start,
                                                        end                                                        
                                                        );
        else if (!manhaton && shadow != nullptr)
            GetNearestPixelIndexKernel_Angle<<<blocks_num, threads_num>>>(ptr_ball_pixel,
                                                        ptr_object_pixel,
                                                        ptr_object_shadow,
                                                        ptr_result, 
                                                        ball_pixel_count, 
                                                        dimension,
                                                        dark_threoshold,
                                                        shadow_threoshold,
                                                        start,
                                                        end   
                                                        );
        else if (manhaton && shadow == nullptr)
            GetNearestPixelIndexKernel_Manhaton_NoShadow<<<blocks_num, threads_num>>>(ptr_ball_pixel, 
                                                         ptr_object_pixel,
                                                         ptr_gray_scale_weight, 
                                                         ptr_result,
                                                         ball_pixel_count,
                                                         dimension, 
                                                         dark_threoshold, 
                                                         start,
                                                         end
                                                         );
        else if (!manhaton && shadow == nullptr)
            GetNearestPixelIndexKernel_Angle_NoShadow<<<blocks_num, threads_num>>>(ptr_ball_pixel,
                                                      ptr_object_pixel,
                                                      ptr_result,
                                                      ball_pixel_count,
                                                      dimension, 
                                                      dark_threoshold,
                                                      start,
                                                      end
                                                      );


                                                         
              // Check for any errors launching the kernel
        cuda_status = cudaGetLastError();
        if (cuda_status != cudaSuccess) 
            goto Error;
    
        // cudaDeviceSynchronize waits for the kernel to finish, and returns
        // any errors encountered during the launch.
        cuda_status = cudaDeviceSynchronize();
        if (cuda_status != cudaSuccess) 
            goto Error;

      }

      // Copy output vector from GPU buffer to host memory.
    cuda_status = cudaMemcpy(result, ptr_result, object_size * sizeof(int), cudaMemcpyDeviceToHost);
    if (cuda_status != cudaSuccess) 
        goto Error;

    return cuda_status;
Error:
    cudaFree(ptr_ball_pixel);
    cudaFree(ptr_object_shadow);
    cudaFree(ptr_object_pixel);
    cudaFree(ptr_gray_scale_weight);
    cudaFree(ptr_result);
    return cuda_status;
}


  __global__ void addKernel(int *c, const int *a, const int *b)
{
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    c[i] = a[i] + b[i];
}

 // Helper function for using CUDA to add vectors in parallel.
cudaError_t addWithCuda(int *c, const int *a, const int *b, unsigned int size)
{
    static const int threadNum = 256;
    
    int *dev_a = 0;
    int *dev_b = 0;
    int *dev_c = 0;
    cudaError_t cudaStatus;
    int blockNum = size / threadNum + ((size % threadNum == 0) ? 0 : 1);
    // Choose which GPU to run on, change this on a multi-GPU system.
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        goto Error;
    }

    // Allocate GPU buffers for three vectors (two input, one output)    .
    cudaStatus = cudaMalloc((void**)&dev_c, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_a, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_b, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    // Copy input vectors from host memory to GPU buffers.
    cudaStatus = cudaMemcpy(dev_a, a, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_b, b, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

      // Launch a kernel on the GPU with one thread for each element.
    addKernel<<<blockNum, threadNum>>>(dev_c, dev_a, dev_b);

    // Check for any errors launching the kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }
    
    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
        goto Error;
    }

    // Copy output vector from GPU buffer to host memory.
    cudaStatus = cudaMemcpy(c, dev_c, size * sizeof(int), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

Error:
    cudaFree(dev_c);
    cudaFree(dev_a);
    cudaFree(dev_b);
    
    return cudaStatus;
}
