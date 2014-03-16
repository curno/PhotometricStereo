#pragma once

#include "float.h"
#include "Math.hpp"
#include "CudaMethods.h"
#include "ImageModel.h"
#include "ModelConfiguration.h"
#include "ModelProgress.h"

class MainWindow;
class PSModel
{
public:

private:
    PSModel() {
        Initialize();   
    }

public:
    static PSModel *CreateModel(const string &dir)
    {
        auto *retval = new PSModel();
        retval->Configuration.ImageSetSource = dir;
        retval->ImageData.NormalCube = ImageDataCubeFactory::CreateNormalCubeFromFiles(dir);
        return retval;
    }
public:
    
    ImageModel ImageData; // The image data and results in the PS process.
    ModelConfiguration Configuration; // The configuration record in the PS process.

    CircleType TargetCircle;

    // Calculate the linear combination parameters of any three lights, with first three lights.
    typedef VectorPtr<double> LightsLinearCombinationsPtr;
    vector<LightsLinearCombinationsPtr> LightsLinearCombinations;

    ModelProgress Progress;
public:
    QSize GetSize() const
    {
        return QSize(ImageData.NormalCube->M, ImageData.NormalCube->N);
    }
    QImage *GetImage(CubeType group, int index)
    {
        auto g = ImageData.GetImageDataCube(group);
        if (g == nullptr || index == -1)
            return nullptr;
        return g->GetQImage(index);
    }
    void HoughCircle()
    {
        int c = ImageData.NormalCube->C;
        vector<IplImage *> copy(c, nullptr);
        for (int i = 0; i < c; ++i)
        {
            const IplImage *image = ImageData.NormalCube->GetCvImage(i);
            copy[i] = cvCreateImage(cvSize(image->width, image->height), image->depth, image->nChannels);
            cvCopy(image, copy[i]);
            cvSmooth(copy[i], copy[i], CV_GAUSSIAN, 3, 0.0, 0.0, 0.0);
            cvSobel(copy[i], copy[i], 0, 1, 3);
        }
        
        const IplImage *image = ImageData.NormalCube->GetCvImage(0);
        IplImage *img_sum = cvCreateImage(cvSize(image->width, image->height), IPL_DEPTH_32F, image->nChannels);
        
        cvZero(img_sum);
        for (int i = 0; i < c; i++)
        {
            IplImage *img = cvCreateImage(cvSize(image->width, image->height), IPL_DEPTH_8U, image->nChannels);
            cvConvertScaleAbs(copy[i], img);
            cvAdd(img, img_sum, img_sum);
            cvReleaseImage(&img);
        }
        IplImage *img_sum_ava = cvCreateImage(cvSize(image->width, image->height), IPL_DEPTH_8U, image->nChannels);
        cvConvertScale(img_sum, img_sum_ava, 1.0 / c);
        cvReleaseImage(&img_sum);
        auto storage = cvCreateMemStorage(0);
        auto *seq = cvHoughCircles(img_sum_ava, storage, CV_HOUGH_GRADIENT, 2, image->width / 5.0, 300, 100, 60, 120);
        for (auto i = copy.begin(); i!= copy.end(); ++i)
            cvReleaseImage(&(*i));
        
        float *p = (float *)cvGetSeqElem(seq, 0);
        Configuration.ObjectLoadingCircle.X = p[0];
        Configuration.ObjectLoadingCircle.Y = p[1];
        Configuration.ObjectLoadingCircle.Z = p[2];
        cvReleaseMemStorage(&storage);
    }

    PixelIndex GetNearestPixelPosition(int x, int y)
    {
        PixelInfo &p = GetNeareastPixel(x, y);
        if (p.IsInvalid)
            return PixelIndex(-1, -1);
        return PixelIndex(p.Index.X, p.Index.Y);
    }

    PixelIndex GetNearestPixelPositionConsideringShadow(int x, int y)
    {
        PixelInfo &p = GetNearestPixelConsideringShadow(x, y);
        if (p.IsInvalid)
            return PixelIndex(-1, -1);
        return PixelIndex(p.Index.X, p.Index.Y);
    }
    PixelInfo &GetNearestPixelConsideringShadow(int x, int y)
    {
        int count = ImageData.BallPixels->size();
        if (count == 0)
            return PixelInfo::InvalidPixel;
        auto v = ImageData.NormalCube->GetData(y, x);
        auto shadow = ImageData.ShadowCube->GetData(y, x);

        auto &p = GetNeareastPixelConsideringShadow(v, shadow, 20, GrayScaleWeight);
        return p;
    }
    PixelInfo &GetNeareastPixel(int x, int y)
    {
        int count = ImageData.BallPixels->size();
        if (count == 0)
            return PixelInfo::InvalidPixel;
        auto v = ImageData.NormalCube->GetData(y, x);
        return GetNeareastPixel(v);
    }

    PixelInfo &GetNeareastPixel(const NormalType *vector)
    {
        double min = DBL_MAX;
        int min_index = -1;
        int count = ImageData.BallPixels->size();
        for (int i = 0; i < count; i++)
        {
            double distance;
            if (Configuration.PixelDistance == ModelConfiguration::PixelDistanceMethod::Manhaton_Method)
                distance = ImageData.BallPixels[i].GetManhatonDistance(vector, false, GrayScaleWeight);
            else if (Configuration.PixelDistance == ModelConfiguration::PixelDistanceMethod::Angle_Method)
                distance = ImageData.BallPixels[i].GetAngleDistance(vector);
            if (distance < min)
            {
                min = distance;
                min_index = i;
            }
        }
        if (min_index < 0 || min == DBL_MAX)
            return PixelInfo::InvalidPixel;
        return ImageData.BallPixels[min_index];
    }

    PixelInfo &GetNeareastPixelConsideringShadow(const NormalType *vector, const ShadowType *shadow, double therdhold, 
        double *scale_weight)
    {
        double min = DBL_MAX;
        int min_index = -1;
        int count = ImageData.BallPixels->size();
        for (int i = 0; i < count; i++)
        {
            double distance;
            if (Configuration.PixelDistance == ModelConfiguration::PixelDistanceMethod::Manhaton_Method)
                distance = ImageData.BallPixels[i].GetManhationDistanceConsideringShadow(vector, false, shadow, therdhold, scale_weight);
            else if (Configuration.PixelDistance == ModelConfiguration::PixelDistanceMethod::Angle_Method)
                distance = ImageData.BallPixels[i].GetAngleDistanceConsideringShadow(vector, shadow, therdhold);
            if (distance < min)
            {
                min = distance;
                min_index = i;
            }
        }
        if (min_index < 0 || min == DBL_MAX)
            return PixelInfo::InvalidPixel;
        return ImageData.BallPixels[min_index];
    }

    void SetTargetCircle(const CircleType &circle)
    {
        TargetCircle = circle;
    }
public:

    double ComputeAverageError()
    {
        int object_count = ImageData.ObjectPixels->size();
        int target_count = ImageData.TargetBallPixels->size();
        double angle_sum = 0;
        for (int i = 0; i < object_count; i++)
        {
            PixelInfo &object = ImageData.ObjectPixels[i];
            bool found = false;
            for (int j = 0; j < target_count; j++)
            {
                PixelInfo &target = ImageData.TargetBallPixels[j];
                if (object.Index == target.Index)
                {
                    found = true;
                    angle_sum += std::acos(max(-1.0, min(1.0, object.Normal * target.Normal)));
                    break;
                }
            }
            if (!found)
                angle_sum += PI;
        }
        angle_sum /= object_count;
        return angle_sum;
    }

    void LoadBallPixels()
    {
        LoadBallPixelsInternal(Configuration.ObjectLoadingCircle, ImageData.BallPixels);

        // progress
        // TODO
    }

    void LoadTargetBallPixels()
    {
        LoadBallPixelsInternal(TargetCircle, ImageData.TargetBallPixels);
        // progress
        // TODO
    }

    void LoadObjectPixelNormals()
    {
        // progress
        // TODO
        static const int LoadBallPixelProgress = 20;
        if (Configuration.UsingCuda)
            Progress.Min = Progress.Max = Progress.Current =  -1;
        else
        {
            Progress.Min = -LoadBallPixelProgress;
            Progress.Current = -LoadBallPixelProgress;
            Progress.Max = Configuration.ObjectLoadingRegion.height() - 1;
        }
        Progress.IsAlive = true;

        QTime time;
        time.start();
        LoadBallPixels();
        
        Progress.Current = 0;
        // configuration


        LoadObjectPixelNormalsInternal();
        Configuration.ElapsedMiliSeconds = time.elapsed();
    }

    void LoadObjectDepth() 
    {
        LoadObjectDepthInternal();

        // configuration
        // Nothing to do

        // progress
        // TODO
    }

    void SmoothObjectNormalField(int count)
    {
        // configuration

        // progress
        // TODO

        // copy image set
        PixelInfoSet copy = PixelInfoSet(ImageData.ObjectPixels->size());
        copy->assign(ImageData.ObjectPixels->begin(), ImageData.ObjectPixels->end());
        ImageData.ObjectPixels = copy;
        for (int i = 0; i < count; ++i)
        {
            SmoothObjectNormalFieldInternal(ImageData.ObjectPixels, Configuration.ObjectLoadingRegion.width(), Configuration.ObjectLoadingRegion.height());
        }

    }
    /*// This method is same as the one below, not very effective.
    //void CreateShadowRemovedImagesUsingLightingDirections()
    //{
    //    int dimension = ImageData.NormalCube->C;
    //    vector<IplImage *> images;

    //    int w = ImageData.NormalCube->M;
    //    int h = ImageData.NormalCube->N;

    //    for (int p = 0; p < dimension; ++p)
    //    {
    //        IplImage *image = cvCreateImage(cvSize(w, h), ImageData.NormalCube->GetCvImage(p)->depth, ImageData.NormalCube->GetCvImage(p)->nChannels);
    //        images.push_back(image);
    //    }

    //    double *A = new double[dimension * dimension];
    //    double *b = new double[dimension];
    //    for (int i = 0; i < dimension; ++i)
    //    {
    //        uvec3 direction = ImageData.LightningDirections->at(i);
    //        A[i] = direction.X;
    //        A[i + dimension] = direction.Y;
    //        A[i + dimension + dimension] = direction.Z;
    //        for (int j = 3; j < dimension; ++j)
    //        {
    //            if (j == i)
    //                A[j * dimension + i] = 1.0;
    //            else
    //                A[j * dimension + i] = 0.0;
    //        }
    //    }
    //    for (int i = 0; i < h; ++i)
    //    {
    //        for (int j = 0; j < w; ++j)
    //        {
    //            NormalType *data = ImageData.NormalCube->GetData(i, j);
    //            if (ImageData.CreatePixelInfo(j, i).DarkPixel())
    //            {
    //                for (int p = 0; p < dimension; ++p)
    //                    ImageDataCubeFactory::SetGrayScaleAt(images[p], j, i, 0);
    //            }
    //            else
    //            {
    //                for (int p = 0; p < dimension; ++p)
    //                    b[p] = data[p];
    //                auto result = LinearLeastSquare(A, b, dimension, dimension);
    //                ImageDataCubeFactory::SetGrayScaleAt(images[0], j, i, data[0]);
    //                ImageDataCubeFactory::SetGrayScaleAt(images[1], j, i, data[1]);
    //                ImageDataCubeFactory::SetGrayScaleAt(images[2], j, i, data[2]);
    //                for (int p = 3; p < dimension; ++p)
    //                    ImageDataCubeFactory::SetGrayScaleAt(images[p], j, i, data[p] - result[p]);
    //            }
    //            
    //            
    //            // normal received. // TODO
    //            
    //        }
    //    }
    //    ImageData.DifferenceCube = ImageDataCubeFactory::CreateNormalCubeFromCvImages<ImageDataCubeType<DifferenceCube>::Type>(images);
    //    delete [] A;
    //    delete [] b;
    //}

    // This is a very naive method, which only use the mid three gray-scale to caculate the other n-3 gray scale.
    //void CreateShadowRemovedImagesUsingLightingDirections()
    //{
    //    int dimension = ImageData.NormalCube->C;
    //    vector<IplImage *> images;

    //    int w = ImageData.NormalCube->M;
    //    int h = ImageData.NormalCube->N;

    //    for (int p = 0; p < dimension; ++p)
    //    {
    //        IplImage *image = cvCreateImage(cvSize(w, h), ImageData.NormalCube->GetCvImage(p)->depth, ImageData.NormalCube->GetCvImage(p)->nChannels);
    //        images.push_back(image);
    //    }

    //    double *A = new double[dimension * dimension];
    //    for (int i = 0; i < dimension; ++i)
    //    {
    //        uvec3 direction = ImageData.LightningDirections->at(i);
    //        A[i] = direction.X;
    //        A[i + dimension] = direction.Y;
    //        A[i + dimension + dimension] = direction.Z;
    //        for (int j = 3; j < dimension; ++j)
    //        {
    //            if (j == i)
    //                A[j * dimension + i] = 1.0;
    //            else
    //                A[j * dimension + i] = 0.0;
    //        }
    //    }
    //    for (int i = 0; i < h; ++i)
    //    {
    //        for (int j = 0; j < w; ++j)
    //        {
    //            NormalType *data = ImageData.NormalCube->GetData(i, j);
    //            int idx[3];
    //            double A[9];
    //            double b[3];
    //            GetMiddleThreeIndex(data, dimension, idx[0], idx[1], idx[2]);
    //            for (int ii = 0; ii < 3; ++ii)
    //            {
    //                A[ii] = ImageData.LightningDirections->at(idx[ii]).X;
    //                A[ii + 3] = ImageData.LightningDirections->at(idx[ii]).Y;
    //                A[ii + 6] = ImageData.LightningDirections->at(idx[ii]).Z;
    //            }
    //            b[0] = data[idx[0]];
    //            b[1] = data[idx[1]];
    //            b[2] = data[idx[2]];
    //            auto result = LinearLeastSquare(A, b, 3, 3);
    //            vec3 normal(result[0], result[1], result[2]);
    //            // normal received. // TODO
    //            for (int p = 0; p < dimension; ++p)
    //                ImageDataCubeFactory::SetGrayScaleAt(images[p], j, i, normal * ImageData.LightningDirections->at(p));
    //        }
    //    }
    //    ImageData.DifferenceCube = ImageDataCubeFactory::CreateNormalCubeFromCvImages<ImageDataCubeType<DifferenceCube>::Type>(images);
    //}*/

    void CreateShadowRemovedImages()
    {
        WeightsCache_.clear();
        if (Configuration.ShadowDetection == ModelConfiguration::ShadowDetectionMethod::PerPixel_Method)
            CreateShadowRemovedImagesPerPixel();
        else if (Configuration.ShadowDetection == ModelConfiguration::ShadowDetectionMethod::ThreeNoShadow_Method)
            CreateShadowRemovedImagesByThreeNoShadow();
        else if (Configuration.ShadowDetection == ModelConfiguration::ShadowDetectionMethod::Ignore_Method)
        {
            // remove shadow images.
            ImageData.ShadowCube = nullptr;
            ImageData.DifferenceCube = nullptr;
            
        }
    }

private:
    //void RefineObjectBoundaryPixels()
    //{
    //    int size = ImageData.ObjectPixels->size();
    //    for (auto i = ImageData.ObjectPixels->begin(); i != ImageData.ObjectPixels->end(); ++i)
    //    {
    //        auto p = *i;
    //        if ((*i).BoundaryPixel())
    //        {
    //            uvec3 n = i->Normal;
    //            i->Normal = uvec3(n.X, n.Y, 0);
    //        }
    //    }
    //}
    void CalcLinearCombination()
    {
        int light_count = ImageData.NormalCube->C;
        LightsLinearCombinations.clear();
        int indices[] = {0, 1, 2};
        for (int k = 3; k < light_count; k++)
        {
            vec3 v;
            CalcLinearCombinationInternal(indices, k, v);
            LightsLinearCombinationsPtr t(3);
            t[0] = v.X;
            t[1] = v.Y;
            t[2] = v.Z;
            LightsLinearCombinations.push_back(t);
        }
    }

    template <typename T>
    struct ScaleWithIndex
    {
        T v;
        int i;
    };
    // TODO
    // This function can be accelerated by using logn + n algorithm. 
    // Don't know if necessary.
    static void GetMiddleThreeIndex(NormalType *data, int dimension, int *ii)
    {

        ii[0] = 0;
        for (int i = 1; i < dimension; ++i)
        {
            if (data[i] > data[ii[0]])
            {
                ii[2] = ii[1];
                ii[1] = ii[0];
                ii[0] = i;
            }
            else if (i < 2 || data[i] > data[ii[1]])
            {
                ii[2] = ii[1];
                ii[1] = i;
            }
            else if (i < 3 || data[i] > data[ii[2]])
            {
                ii[2] = i;
            }
        }

        /*ScaleWithIndex<NormalType> *data_i = new ScaleWithIndex<NormalType>[dimension];
        for (int i = 0; i < dimension; ++i)
        {
        data_i[i].v = data[i];
        data_i[i].i = i;
        }
        sort(data_i, data_i + dimension, [](const ScaleWithIndex<NormalType> &i1, const ScaleWithIndex<NormalType> &i2) { return i1.v < i2.v; });
        ii[0] = data_i[dimension / 2 - 1].i;
        ii[1] = data_i[dimension / 2].i;
        ii[2] = data_i[dimension / 2 + 1].i;
        delete [] data_i;*/
        sort(ii, ii + 3);
    }
    void SampleLightingVectors(double *samples, int *indices, int sample_size)
    {
        int side = sample_size;
        float factor = 0.3;
        int X = Configuration.ShadowDetectionCircle.X;
        int Y = Configuration.ShadowDetectionCircle.Y;
        double R = Configuration.ShadowDetectionCircle.Z;
        int step = R * factor * 2 / (side + 1);

        for (int k = 0; k < 3; k++)
        {
            int index = indices[k];
            int count = 0;
            for (int i = 0; i < side; i++)
            {
                int y = (int)(Y - R * factor + i * step);
                for (int j = 0; j < side; j++)
                {
                    int x = (int)(X - R * factor + j * step);
                    samples[k * sample_size * sample_size + count] = ImageData.NormalCube->GetData(y, x)[index];
                    count++;
                }
            }
        }

    }

    void Initialize()
    {
        InitGrayScaleWeight();
    }
    void LoadBallPixelsInternal(const CircleType &circle, PixelInfoSet &pixels_p)
    {
        vector<PixelInfo> *pixels = new vector<PixelInfo>; 
        int X = circle.X;
        int Y = circle.Y;
        double R = circle.Z;
        int count = ImageData.NormalCube->C;
        for (int h = 0; h <= R; h++)
        {
            double w_total = std::sqrt(R * R - h * h);
            for (int w = 0; w < w_total; w++)
            {
                double z = std::sqrt(w_total * w_total - w * w);

                PixelInfo p1;
                p1.Index.X = (X - w);
                p1.Index.Y = (Y - h);
                p1.Normal = uvec3(-w, -h, z);
                p1.SetImageData(&ImageData);
                pixels->push_back(p1);

                PixelInfo p2;
                p2.Index.X = (X - w);
                p2.Index.Y = (Y + h);
                p2.Normal = uvec3(-w, h, z);
                p2.SetImageData(&ImageData);
                pixels->push_back(p2);

                PixelInfo p3;
                p3.Index.X = (X + w);
                p3.Index.Y = (Y + h);
                p3.Normal = uvec3(w, h, z);
                p3.SetImageData(&ImageData);
                pixels->push_back(p3);

                PixelInfo p4;
                p4.Index.X = (X + w);
                p4.Index.Y = (Y - h);
                p4.Normal = uvec3(w, -h, z);
                p4.SetImageData(&ImageData);
                pixels->push_back(p4);
            }
        }
        pixels_p.SetData(pixels);
    }

    void LoadObjectPixelNormals_CPU_Shadow() 
    {
        int w = Configuration.ObjectLoadingRegion.width();
        int h = Configuration.ObjectLoadingRegion.height();
        int x = Configuration.ObjectLoadingRegion.x();
        int y = Configuration.ObjectLoadingRegion.y();
        int count = ImageData.NormalCube->C;
        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                PixelInfo p1;
                p1.Index.X = (j + x);
                p1.Index.Y = (i + y);
                p1.Position.X = j;
                p1.Position.Y = i;
                p1.SetImageData(&ImageData);
                NormalType *data = p1.GetData<NormalType>(NormalCube);
                bool flag_dark = true;
                for (int k = 0; k < count; k++)
                {
                    if (data[k] > 20)
                    {
                        flag_dark = false;
                        break;
                    }
                }
                if (flag_dark)
                {

                    p1.Normal = uvec3(0, 0, 1);
                }
                else
                {
                    auto *shadow = ImageData.ShadowCube->GetData(i + y, j + x);
                    PixelInfo &p = GetNeareastPixelConsideringShadow(data, shadow, Configuration.ShadowThreshold, GrayScaleWeight);
                    if (!p.IsInvalid)
                        p1.Normal = p.Normal;
                    else
                    {
                        p1.Normal = uvec3(0, 0, 1);
                    }
                }

                ImageData.ObjectPixels->push_back(p1);
            }
            // progress
            Progress.Current = i;
            if (!Progress.IsAlive)
                break;
        }
    }

    // NOTE: This function is pixel type specified to double
    void LoadObjectPixelNormals_CUDA()
    {
        int w = Configuration.ObjectLoadingRegion.width();
        int h = Configuration.ObjectLoadingRegion.height();
        int x = Configuration.ObjectLoadingRegion.x();
        int y = Configuration.ObjectLoadingRegion.y();
        int dimension = ImageData.NormalCube->C;
        int ball_pixel_count = ImageData.BallPixels->size();

        int object_size = w * h;
        int ball_pixel_lighning_vector_size = dimension * ball_pixel_count;
        int object_pixel_lightning_vector_size = dimension * object_size;
        int object_pixel_shadow_vector_size = object_pixel_lightning_vector_size;
        const int gray_scale_weight_size = 256;

        // Copy data together in memory.
        // I wonder if there is a way avoid doing this, just copy the data directly to GPU's memory.
        double *ptr_ball_pixel = new double[ball_pixel_lighning_vector_size], 
            *ptr_object_pixel = new double[object_pixel_lightning_vector_size];
        double *ptr_object_shadow = nullptr;
        

        int *ptr_result = new int[object_size];
        double *ptr_gray_scale_weight = GrayScaleWeight;

        // ball pixel
        double *ptr_ball_pixel_current = ptr_ball_pixel;
        for (auto i = ImageData.BallPixels->begin(); i != ImageData.BallPixels->end(); ++i)
        {
            for (int j = 0; j < dimension; ++j)
                ptr_ball_pixel_current[j] = (*i).GetData<NormalType>(NormalCube)[j];
            ptr_ball_pixel_current += dimension;
        }

        // object pixel
        double *ptr_object_pixel_current = ptr_object_pixel;
        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                PixelInfo p1;
                p1.Index.X = (j + x);
                p1.Index.Y = (i + y);
                p1.Position.X = j;
                p1.Position.Y = i;
                p1.SetImageData(&ImageData);

                ImageData.ObjectPixels->push_back(p1);
                if (Configuration.ShadowUse == ModelConfiguration::ShadowUseMethod::Ignore_Shadow)
                {
                    auto data = p1.GetData<NormalType>(NormalCube);
                    ::std::copy(data, data + dimension, ::stdext::checked_array_iterator<double *>(ptr_object_pixel_current, dimension));
                }
                else
                {
                    auto data = p1.GetData<DifferenceType>(DifferenceCube);
                    for (int index = 0; index < dimension; ++index)
                        ptr_object_pixel_current[index] = min(255.0, max(0.0, data[index]));
                }
                
               
                ptr_object_pixel_current += dimension;
            }
        }

        // shadow
        if (Configuration.ShadowDetection != ModelConfiguration::ShadowDetectionMethod::Ignore_Method && Configuration.ShadowUse == ModelConfiguration::ShadowUseMethod::Ignore_Shadow)
        {
            ptr_object_shadow = new double[object_pixel_shadow_vector_size];
            double *ptr_object_shadow_current = ptr_object_shadow;
            for (int i = 0; i < h; i++)
            {
                for (int j = 0; j < w; j++)
                {
                    auto shadow = ImageData.ShadowCube->GetData(i + y, j + x);
                    memcpy(ptr_object_shadow_current, shadow, sizeof(double) * dimension);
                    ptr_object_shadow_current += dimension;
                }
            }
        }

        if (Configuration.PixelDistance == ModelConfiguration::PixelDistanceMethod::Manhaton_Method)
            CudaGetNearestPixelIndex(ptr_ball_pixel, ptr_object_pixel, ptr_object_shadow, ptr_gray_scale_weight, 
                 ptr_result, ball_pixel_count, h, w, dimension, 20, Configuration.ShadowThreshold, true);
        else
            CudaGetNearestPixelIndex(ptr_ball_pixel, ptr_object_pixel, ptr_object_shadow, ptr_gray_scale_weight, 
            ptr_result, ball_pixel_count, h, w, dimension, 20, Configuration.ShadowThreshold, false);

        for (int i = 0; i < ImageData.ObjectPixels->size(); ++i)
        {
            if (ptr_result[i] < 0)
                ImageData.ObjectPixels[i].Normal = uvec3(0, 0, 1);
            else
                ImageData.ObjectPixels[i].Normal = ImageData.BallPixels[ptr_result[i]].Normal;
        }

        delete [] ptr_ball_pixel;
        delete [] ptr_object_shadow;
        delete [] ptr_object_pixel;
        delete [] ptr_result;
    }
    void InitGrayScaleWeight()
    {
        for (int i = 0; i < 256; i++)
        {
            GrayScaleWeight[i] = 1 + 3 * std::cos(i / 510.0 * PI);
        }
    }

    void CalcLinearCombinationInternal(int *left, int right, vec3 &result)
    {
        static const int SampleSize = 3;

        int light_count = ImageData.NormalCube->C;

        WeightToken wt;
        wt.Index[0] = left[0];
        wt.Index[1] = left[1];
        wt.Index[2] = left[2];
        wt.Index[3] = right;
        auto i = WeightsCache_.find(wt);
        if (i != WeightsCache_.end())
        {
            result = i->second;
            return;
        }

        double *A = new double[SampleSize * SampleSize * 3];

        SampleLightingVectors(A, left, SampleSize);
        float factor = 0.7;
        int X = Configuration.ShadowDetectionCircle.X;
        int Y = Configuration.ShadowDetectionCircle.Y;
        double R = Configuration.ShadowDetectionCircle.Z;
        int step = R * factor * 2 / (SampleSize + 1);

        double *b = new double[SampleSize * SampleSize];
        int count = 0;
        for (int i = 0; i < SampleSize; i++)
        {
            int y = (int)(Y - R * factor + i * step);
            for (int j = 0; j < SampleSize; j++)
            {
                int x = (int)(X - R * factor + j * step);
                b[count++] = ImageData.NormalCube->GetData(y, x)[right];
            }
        }
        auto r = LinearLeastSquare(A, b, SampleSize * SampleSize, 3);
        result.X = r[0];
        result.Y = r[1];
        result.Z = r[2];
        delete [] A;
        delete [] b;
        WeightsCache_[wt] = result;
    }

    void SmoothObjectNormalFieldInternal(PixelInfoSet pixels, int w, int h)
    {
        int current = 0;
        for (int i = 0; i < h; ++i)
        {
            for (int j = 0; j < w; ++j)
            {
                vec3 v = pixels[current].Normal * 4;
                
                if (i > 0)
                    v += pixels[(i - 1) * w + j].Normal * 2.0;
                
                if (i < h - 1)
                    v += pixels[(i + 1) * w + j].Normal * 2.0;
                
                if (j > 0)
                    v += pixels[i * w + j - 1].Normal * 2.0;

                if (j < w - 1)
                    v += pixels[i * w + j + 1].Normal * 2.0;
               
                if (i > 0 && j > 0)
                    v += pixels[(i - 1) * w + j - 1].Normal;
               
                if (i > 0 && j < w - 1)
                    v += pixels[(i - 1) * w + j + 1].Normal;
               
                if (i < h - 1 && j > 0)
                    v += pixels[(i + 1) * w + j - 1].Normal;
                
                if (i < h - 1 && j < w - 1)
                    v += pixels[(i + 1) * w + j + 1].Normal;
               
                pixels[current].SetNormal(uvec3(v.X, v.Y, v.Z));
                current++;
            }
        }
    }

    void LoadObjectPixelNormals_CPU_NoShadow(CubeType type)
    {
        ImageData.ObjectPixels = PixelInfoSet();
        int w = Configuration.ObjectLoadingRegion.width();
        int h = Configuration.ObjectLoadingRegion.height();
        int x = Configuration.ObjectLoadingRegion.x();
        int y = Configuration.ObjectLoadingRegion.y();
        int count = ImageData.NormalCube->C;
        NormalType *data2 = new NormalType[count];
        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                PixelInfo p1;
                p1.Index.X = j + x;
                p1.Index.Y = i + y;
                p1.Position.X = j;
                p1.Position.Y = i;
                p1.SetImageData(&(this->ImageData));
                bool flag_dark = true;
                
                NormalType *data = nullptr;
                if (type == NormalCube)
                    data = p1.GetData<NormalType>(type);
                else if (type == DifferenceCube)
                {
                    data = data2;
                    DifferenceType *tmp = p1.GetData<DifferenceType>(type);
                    for (int tmp_i = 0; tmp_i < count; ++tmp_i)
                        data[tmp_i] = max(0, min(255, static_cast<NormalType>(tmp[tmp_i])));
                }
                for (int k = 0; k < count; k++)
                {
                    if (data[k] > 20)
                    {
                        flag_dark = false;
                        break;
                    }
                }
                if (flag_dark)
                {
                    p1.Normal = uvec3(0, 0, 1);
                }
                else
                {
                    PixelInfo &p = GetNeareastPixel(data);
                    if (!p.IsInvalid)
                        p1.Normal = p.Normal;
                    else
                    {
                        p1.Normal = uvec3(0, 0, 1);
                    }
                }

                ImageData.ObjectPixels->push_back(p1);
            }
            // progress
            Progress.Current = i;
            if (!Progress.IsAlive)
                break;
        }
        delete [] data2;

    }

    void LoadObjectPixelNormalsInternal()
    {
        ImageData.ObjectPixels = PixelInfoSet();

        if (Configuration.UsingCuda)
            LoadObjectPixelNormals_CUDA();
        else 
        {
            if (Configuration.ShadowDetection == ModelConfiguration::ShadowDetectionMethod::Ignore_Method)
                LoadObjectPixelNormals_CPU_NoShadow(NormalCube);
            else
            {
                if (Configuration.ShadowUse == ModelConfiguration::ShadowUseMethod::Ignore_Shadow)
                    LoadObjectPixelNormals_CPU_Shadow();
                else
                    LoadObjectPixelNormals_CPU_NoShadow(DifferenceCube);
            }
        }
    }

    // Calculate the depth of the object pixels
    // as well as the actual normal of the pixels
    void LoadObjectDepthInternal() 
    {
        PhotometricStereoMathInitialize();
        int w = Configuration.ObjectLoadingRegion.width();
        int h = Configuration.ObjectLoadingRegion.height();

        mxArray *dzdx = mxCreateDoubleMatrix(h, w, mxREAL);
        mxArray *dzdy = mxCreateDoubleMatrix(h, w, mxREAL);

        mxArray *input[2] = { dzdx, dzdy };
        mxArray *output[1];

        double *dzdx_data = static_cast<double *>(mxGetData(dzdx));
        double *dzdy_data = static_cast<double *>(mxGetData(dzdy));
        int index = 0;
        for (int i = 0; i < w; i++)
        {
            for (int j = 0; j < h; j++)
            {
                PixelInfo &p = ImageData.ObjectPixels[j * w + i];
                if (p.Normal.Z == 0.0)
                {
                    dzdy_data[index] = -p.Normal.Y < 0 ? FLT_MIN : FLT_MAX;
                    dzdx_data[index] = -p.Normal.X < 0 ? FLT_MIN : FLT_MAX;
                }
                else
                {
                    dzdx_data[index] = -p.Normal.X / p.Normal.Z;
                    dzdy_data[index] = -p.Normal.Y / p.Normal.Z;
                }

                index++;
            }
        }

        mlxFrankotchellappa(1, output, 2, input);

        double *retval = static_cast<double *>(mxGetData(output[0]));
        index = 0;
        for (int i = 0; i < w; i++)
        {
            for (int j = 0; j < h; j++)
            {
                PixelInfo &p = ImageData.ObjectPixels[j * w + i];
                p.Position.Z = retval[index++] / 6.0;
            }
        }

        for (int i = 0; i < w; i++)
        {
            for (int j = 0; j < h; j++)
            {
                vec3 r;
                vec3 p = ImageData.ObjectPixels[j * w + i].Position;
                if (i > 0 && j > 0)
                {
                    vec3 n = (ImageData.ObjectPixels[(j - 1) * w + i].Position - p) ^ (ImageData.ObjectPixels[j * w + i - 1].Position - p);
                    r += n / n.GetLength();
                }
                if (i > 0 && j < h - 1)
                {
                    vec3 n = (ImageData.ObjectPixels[j * w + i - 1].Position - p) ^ (ImageData.ObjectPixels[(j + 1) * w + i].Position - p);
                    r += n / n.GetLength();
                }
                if (i < w - 1 && j < h - 1)
                {
                    vec3 n = (ImageData.ObjectPixels[(j + 1) * w + i].Position - p) ^ (ImageData.ObjectPixels[j * w + i + 1].Position - p);
                    r += n / n.GetLength();
                }
                if (i < w - 1 && j > 0)
                {
                    vec3 n = (ImageData.ObjectPixels[j * w + i + 1].Position - p) ^ (ImageData.ObjectPixels[(j - 1) * w + i].Position - p);
                    r += n / n.GetLength();
                }
                // here, use the negative flag '-', because the y-axis of the object space is from up to down of the screen.
                ImageData.ObjectPixels[j * w + i].ActualNormal = -uvec3(r.X, r.Y, r.Z);
            }
        }
    }
    void CalcLigtningDirections(int sample_size = 3)
    {
        ImageData.LightningDirections = make_shared<vector<uvec3>>();
        int dimension = ImageData.NormalCube->C;
        double *A = new double[sample_size * 3];
        double *As[] = {new double[sample_size], new double[sample_size], new double[sample_size]};
        double *b = new double[sample_size];
        int ball_size = ImageData.BallPixels->size();
        int step = ball_size / (sample_size + 1);

        for (int i = 0; i < dimension; ++i)
        {
            int sample_count = 0;
            for (int j = 0; j < sample_size; ++j)
            {
                auto &pixel = ImageData.BallPixels[step * j + step / 2];
                if (pixel.GetData<NormalType>(NormalCube)[i] < 10)
                    continue;
                uvec3 normal = pixel.Normal;
                As[0][sample_count] = normal.X;
                As[1][sample_count] = normal.Y;
                As[2][sample_count] = normal.Z;
                b[sample_count] = ImageData.NormalCube->GetData(pixel.Index.Y, pixel.Index.X)[i];
                sample_count++;

            }
            memcpy(A, As[0], sizeof(double) * sample_count);
            memcpy(A + sample_count, As[1], sizeof(double) * sample_count);
            memcpy(A + sample_count * 2, As[2], sizeof(double) * sample_count);
            auto result = LinearLeastSquare(A, b, sample_count, 3);
            ImageData.LightningDirections->push_back(uvec3(result[0], result[1], result[2]));
        }
        delete [] A;
        delete [] As[0];
        delete [] As[1];
        delete [] As[2];
        delete [] b;
    }

    void CreateDifferenceShadowImages()
    {
        int c = ImageData.NormalCube->C;
        vector<IplImage *> images;
        for (int i = 0; i < c; i++)
        {
            try
            {
                auto *image = ImageData.NormalCube->GetCvImage(i);
                auto *image_shadow_removed = ImageData.DifferenceCube->GetCvImage(i);
                IplImage *result = cvCreateImage(cvSize(image->width, image->height), IPL_DEPTH_64F, image->nChannels);
                cvSub(image, image_shadow_removed, result);
                cvAbs(result, result);
                images.push_back(result);
            }

            catch (cv::Exception &e)
            {
                qDebug(e.msg.c_str());
            }
        }
        ImageData.ShadowCube = ImageDataCubeFactory::CreateNormalCubeFromCvImages<ImageDataCubeType<ShadowCube>::Type>(images);
    }

    void CreateShadowRemovedImagesByThreeNoShadowInternal() 
    {
        CalcLinearCombination();
        vector<IplImage *> images;
        IplImage *o0 = ImageData.NormalCube->GetCvImage(0);
        IplImage *i0 = cvCreateImage(cvSize(o0->width, o0->height), o0->depth, o0->nChannels);
        cvCopy(o0, i0);
        IplImage *o1 = ImageData.NormalCube->GetCvImage(1);
        IplImage *i1 = cvCreateImage(cvSize(o1->width, o1->height), o1->depth, o1->nChannels);
        cvCopy(o1, i1);
        IplImage *o2 = ImageData.NormalCube->GetCvImage(2);
        IplImage *i2 = cvCreateImage(cvSize(o2->width, o2->height), o2->depth, o2->nChannels);
        cvCopy(o2, i2);
        images.push_back(i0);
        images.push_back(i1);
        images.push_back(i2);

        int c = ImageData.NormalCube->C;
        IplImage *zero = cvCreateImage(cvSize(o0->width, o0->height), IPL_DEPTH_64F, o0->nChannels);
        cvSetZero(zero);

        for (int i = 3; i < c; i++)
        {
            auto &weights = LightsLinearCombinations.at(i - 3);
            auto *image = ImageData.NormalCube->GetCvImage(0);
            IplImage *image_add = cvCreateImage(cvSize(image->width, image->height), IPL_DEPTH_64F, image->nChannels);
            cvAddWeighted(ImageData.NormalCube->GetCvImage(0), weights[0], ImageData.NormalCube->GetCvImage(1), weights[1], 0.0, image_add);
            cvAddWeighted(ImageData.NormalCube->GetCvImage(2), weights[2], image_add, 1.0, 0.0, image_add);
            cvMax(zero, image_add, image_add);
            images.push_back(image_add);
        }
        ImageData.DifferenceCube = ImageDataCubeFactory::CreateNormalCubeFromCvImages<ImageDataCubeType<DifferenceCube>::Type>(images);
        CreateDifferenceShadowImages();
    }

    void CreateShadowRemovedImagesPerPixelInternal() 
    {
        int dimension = ImageData.NormalCube->C;
        vector<IplImage *> images;

        int w = ImageData.NormalCube->M;
        int h = ImageData.NormalCube->N;

        for (int p = 0; p < dimension; ++p)
        {
            IplImage *image = cvCreateImage(cvSize(w, h), ImageData.NormalCube->GetCvImage(p)->depth, ImageData.NormalCube->GetCvImage(p)->nChannels);
            images.push_back(image);
        }

        for (int i = 0; i < h; ++i)
        {
            for (int j = 0; j < w; ++j)
            {
                if (ImageData.CreatePixelInfo(j, i).DarkPixel())
                {
                    for (int p = 0; p < dimension; ++p)
                        ImageDataCubeFactory::SetGrayScaleAt(images[p], j, i, 0);
                    continue;
                }
                NormalType *data = ImageData.NormalCube->GetData(i, j);
                int idx[3];
                GetMiddleThreeIndex(data, dimension, idx);
                for (int p = 0; p < dimension; ++p)
                {
                    if (p == idx[0] || p == idx[1] || p == idx[2])
                        ImageDataCubeFactory::SetGrayScaleAt(images[p], j, i, data[p]);
                    else
                    {
                        vec3 v;
                        CalcLinearCombinationInternal(idx, p, v);
                        ImageDataCubeFactory::SetGrayScaleAt(images[p], j, i, max(0.0, data[idx[0]] * v.X + data[idx[1]] * v.Y + data[idx[2]] * v.Z));
                    }
                }

            }
        }
        ImageData.DifferenceCube = ImageDataCubeFactory::CreateNormalCubeFromCvImages<ImageDataCubeType<DifferenceCube>::Type>(images);
        CreateDifferenceShadowImages();
    }

    void CreateShadowRemovedImagesByThreeNoShadow()
    {
        CreateShadowRemovedImagesByThreeNoShadowInternal();

        // configuration

        // progress
        // TODO
    }

    void CreateShadowRemovedImagesPerPixel()
    {
        CreateShadowRemovedImagesPerPixelInternal();

        // configuration

        // progress
        // TODO
    }

private:
    // GrayScaleWeight
    double GrayScaleWeight[256];
    struct WeightToken
    {
        int Index[4];
        bool operator==(const WeightToken &other) const
        {
            return Index[0] == other.Index[0] &&
                Index[1] == other.Index[1] &&
                Index[2] == other.Index[2] &&
                Index[3] == other.Index[3];
        }
    };

    struct WeightTokenHasher
    {
        size_t operator()(const WeightToken& t) const {
            std::hash<int> h;
            return (size_t)(h(t.Index[0]) ^ h(t.Index[1]) ^ h(t.Index[2]) ^ h(t.Index[3]));
        }
    };
    unordered_map<WeightToken, vec3, WeightTokenHasher> WeightsCache_;

};

