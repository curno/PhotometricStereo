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
    static const int MaxAngleErrorForColorIndex = 75;
    static QColor GetAngleErrorColor(double angle);
private:
    PSModel();

public:
    static PSModel *CreateModel(const string &dir);
public:
    
    ImageModel ImageData; // The image data and results in the PS process.
    ModelConfiguration Configuration; // The configuration record in the PS process.

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
    bool HoughCircle();
    bool DetectObject();
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
            else if (Configuration.PixelDistance == ModelConfiguration::PixelDistanceMethod::Euclidiean_Method)
                distance = ImageData.BallPixels[i].GetEuclideanDistance(vector, false, GrayScaleWeight);
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
            else if (Configuration.PixelDistance == ModelConfiguration::PixelDistanceMethod::Euclidiean_Method)
                distance = ImageData.BallPixels[i].GetEuclideanDistanceConsideringShadow(vector, false, shadow, therdhold, scale_weight);
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

    pair<QImage, PixelInfoSet> GetReconstructDifference();
public:

    double ComputeAverageError(double &max);

    void LoadBallPixels();

    void LoadTargetBallPixels(CircleType circle);
    void LoadTargetCylinderPixels(int x, int y, int width, int height);
    void LoadTargetConePixels(int bottom, int top, int left, int right);

    PixelInfoSet GetGroundTruth();

    void LoadObjectPixelNormals();

    void LoadObjectDepth();

    void SmoothObjectNormalField(int count, bool only_border);


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

    void CreateShadowRemovedImages();

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
    void CalcLinearCombination();

    template <typename T>
    struct ScaleWithIndex
    {
        T v;
        int i;
    };
    // TODO
    // This function can be accelerated by using logn + n algorithm. 
    // Don't know if necessary.
    static void GetMiddleThreeIndex(NormalType *data, int dimension, int *ii);

    static void GetFirstThreeIndex(NormalType *data, int dimension, int *ii);

    void SampleLightingVectors(double *samples, int *indices, int sample_size);

    void Initialize();
    void LoadBallPixelsInternal(const CircleType &circle, PixelInfoSet &pixels_p);

    void LoadObjectPixelNormals_CPU_Shadow();

    // NOTE: This function is pixel type specified to double
    void LoadObjectPixelNormals_CUDA();
    void InitGrayScaleWeight();

    void CalcLinearCombinationInternal(int *left, int right, vec3 &result);

    void SmoothObjectNormalFieldInternal(PixelInfoSet pixels, int w, int h, bool only_background);

    void LoadObjectPixelNormals_CPU_NoShadow(CubeType type);

    void LoadObjectPixelNormalsInternal();

    // Calculate the depth of the object pixels
    // as well as the actual normal of the pixels
    void LoadObjectDepthInternal(PixelInfoSet pixels, QRect region);

    void LoadActuralNormal(PixelInfoSet pixels, QRect region);

    void CalcLigtningDirections(int sample_size = 3);

    void CreateDifferenceShadowImages();

    void CreateShadowRemovedImagesByThreeNoShadowInternal();

    void CreateShadowRemovedImagesPerPixelInternal();

    void CreateShadowRemovedImagesByThreeNoShadow();

    void CreateShadowRemovedImagesPerPixel();


    PixelInfoSet RectanglizePixelInfoSet(PixelInfoSet pixels, QRect region);
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


    static bool Ugly(const string &dir_name)
    {
        QDir dir(FromStdStringToQString(dir_name));
        QFileInfoList list = dir.entryInfoList();
        foreach (QFileInfo info, list)
            if (info.completeBaseName().endsWith("H"))
                return true;
        return false;

    }
};

