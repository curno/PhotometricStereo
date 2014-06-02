#include "stdafx.h"

#include "PSModel.h"


bool PSModel::HoughCircle()
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
    auto *seq = cvHoughCircles(img_sum_ava, storage, CV_HOUGH_GRADIENT, 3, image->width/2.0, 100, 100, 65, 80);
    for (auto i = copy.begin(); i!= copy.end(); ++i)
        cvReleaseImage(&(*i));

    if (seq->total > 0)
    {
        float *p = (float *)cvGetSeqElem(seq, seq->total - 1);
        Configuration.ObjectLoadingCircle.X = p[0];
        Configuration.ObjectLoadingCircle.Y = p[1];
        Configuration.ObjectLoadingCircle.Z = p[2];
        cvReleaseMemStorage(&storage);
        return true;
    }
    return false;
}

bool PSModel::DetectObject()
{
    int width = ImageData.NormalCube->M;
    int height = ImageData.NormalCube->N;

    // get connected pixel from loading circle
    vector<vector<bool>> flags(height, vector<bool>(width, false));
    std::queue<PixelIndex> current;
    for (int h = 0; h < height; ++h)
    {
        for (int w = 0; w < width; ++w)
        {
            if (flags[h][w])
                continue;
            if ((h - Configuration.ObjectLoadingCircle.Y) * (h - Configuration.ObjectLoadingCircle.Y) + 
                (w - Configuration.ObjectLoadingCircle.X) * (w - Configuration.ObjectLoadingCircle.X) <= Configuration.ObjectLoadingCircle.Z * Configuration.ObjectLoadingCircle.Z)
            {
                flags[h][w] = true;
                current.push(PixelIndex(w, h));
                while (!current.empty())
                {
                    auto p = current.front();
                    current.pop();
                    PixelInfo pi;
                    #define CHECK_AND_PUSH(x, y) \
                        pi = ImageData.CreatePixelInfo(x, y); \
                        if (!pi.IsInvalid && !flags[y][x] && !pi.DarkPixel()) \
                                        { \
                                        flags[y][x] = true; \
                                        current.push(pi.Index); \
                                        }

                    CHECK_AND_PUSH(p.X - 1, p.Y)
                    CHECK_AND_PUSH(p.X + 1, p.Y)
                    CHECK_AND_PUSH(p.X, p.Y - 1)
                    CHECK_AND_PUSH(p.X, p.Y + 1)
                    CHECK_AND_PUSH(p.X - 1, p.Y - 1)
                    CHECK_AND_PUSH(p.X + 1, p.Y + 1)
                    CHECK_AND_PUSH(p.X - 1, p.Y - 1)
                    CHECK_AND_PUSH(p.X + 1, p.Y + 1)
                    #undef CHECK_AND_PUSH
                }
            }
        }
    }
    

    QRect rect;
    bool first = false;
    
    for (int h = 0; h < height; ++h)
        for (int w = 0; w < width; ++w)
        {
            if (flags[h][w])
                continue;
            PixelInfo pi = ImageData.CreatePixelInfo(w, h);
            if (pi.DarkPixel())
                continue;
            if (!first)
            {
                first = true;
                rect = QRect(w, h, 0, 0);
            }
            else
            {
                rect.setLeft(min(w, rect.left()));
                rect.setRight(max(w, rect.right()));
                rect.setBottom(max(h, rect.bottom()));
                rect.setTop(min(h, rect.top()));
            }
        }

        if (first)
        {
            QPoint center = rect.center();
            rect.setSize(QSize(rect.width() * 1.1, rect.height() * 1.1));
            rect.moveCenter(center);
            Configuration.ObjectLoadingRegion = rect;
            return true;
        }
        return false;
}


double PSModel::ComputeAverageError(double &max_value)
{
    int object_count = ImageData.ObjectPixels->size();
    int target_count = ImageData.TargetObjectPixels->size();
    double angle_sum = 0;
    int total_pixel_count = 0;
    max_value = 0;
    for (int i = 0; i < target_count; i++)
    {
        PixelInfo &ball = ImageData.TargetObjectPixels[i];
        bool found = false;
        for (int j = 0; j < object_count; j++)
        {
            PixelInfo &object = ImageData.ObjectPixels[j];
            if (object.Index == ball.Index && !object.DarkPixel())
            {
                found = true;
                double angle = std::acos(max(-1.0, min(1.0, object.Normal * ball.Normal)));
                max_value = max(max_value, angle);
                angle_sum += angle;
                break;
            }
        }
        if (found)
            ++total_pixel_count;
    }
    angle_sum /= total_pixel_count;
    return angle_sum;
}

void PSModel::LoadTargetCylinderPixels(int x, int y, int width, int height)
{
    vector<PixelInfo> *pixels = new vector<PixelInfo>; 

    int half_width = width / 2;
    int x_base = x + half_width;
    for (int w = 0; w <= half_width; ++w)
    {
        for (int h = 0; h <= height; ++h)
        {
            double z = std::sqrt(half_width * half_width - w * w);

            PixelInfo p1;
            p1.Index.X = x_base - w;
            p1.Index.Y = h + y;
            p1.Normal = uvec3(-w, 0, z);
            //p1.Position = vec3(half_width - w, h, z);
            p1.SetImageData(&ImageData);
            pixels->push_back(p1);

            PixelInfo p2;
            p2.Index.X = x_base + w;
            p2.Index.Y = h + y;
            p2.Normal = uvec3(w, 0, z);
            //p2.Position = vec3(half_width + w, h, z);
            p2.SetImageData(&ImageData);
            pixels->push_back(p2);
        }
    }
    this->ImageData.TargetObjectPixels.SetData(pixels);

}

void PSModel::LoadTargetConePixels(int bottom, int top, int left, int right)
{
    vector<PixelInfo> *pixels = new vector<PixelInfo>; 

    int height = bottom - top;
    double R = (right - left) / 2.0;
    int center_x = (left + right) / 2;
    int base_y = top;
    for (int h = 0; h <= height; ++h)
    {
        double r = h * R / height;
        for (int w = 0; w < r; ++w)
        {
            double z = std::sqrt(r * r - w * w);
            PixelInfo p1;
            p1.Index.X = (center_x - w);
            p1.Index.Y = base_y + h;
            p1.Normal = uvec3(-w * h, -z * z - w * w, z * h);
            //p1.Position = vec3(R - w, h, z);
            p1.SetImageData(&ImageData);
            pixels->push_back(p1);

            PixelInfo p2;
            p2.Index.X = (center_x + w);
            p2.Index.Y = base_y + h;
            p2.Normal = uvec3(w * h, - z * z - w * w, z * h);
            //p2.Position = vec3(R + w, h, z);
            p2.SetImageData(&ImageData);
            pixels->push_back(p2);
        }
    }

    this->ImageData.TargetObjectPixels.SetData(pixels);

}

void PSModel::CreateShadowRemovedImagesPerPixel()
{
    CreateShadowRemovedImagesPerPixelInternal();

    // configuration

    // progress
    // TODO
}

void PSModel::CreateShadowRemovedImagesByThreeNoShadow()
{
    CreateShadowRemovedImagesByThreeNoShadowInternal();

    // configuration

    // progress
    // TODO
}

void PSModel::CreateShadowRemovedImagesPerPixelInternal()
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

void PSModel::CreateShadowRemovedImagesByThreeNoShadowInternal()
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

void PSModel::CreateDifferenceShadowImages()
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

void PSModel::CalcLigtningDirections(int sample_size /*= 3*/)
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

void PSModel::LoadObjectDepthInternal(PixelInfoSet pixels, QRect region)
{
    PhotometricStereoMathInitialize();
    int w = region.width();
    int h = region.height();

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
            PixelInfo &p = pixels[j * w + i];
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
            PixelInfo &p = pixels[j * w + i];
            p.Position.Z = retval[index++] / 6.0;
        }
    }

    LoadActuralNormal(pixels, region);

}

void PSModel::LoadObjectPixelNormalsInternal()
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

void PSModel::LoadObjectPixelNormals_CPU_NoShadow(CubeType type)
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

void PSModel::SmoothObjectNormalFieldInternal(PixelInfoSet pixels, int w, int h, bool only_background)
{
    int current = 0;
    for (int i = 0; i < h; ++i)
    {
        for (int j = 0; j < w; ++j)
        {
            if (only_background && !pixels[current].BoundaryPixel() && !pixels[current].DarkPixel())
            {
                current++;
                continue;
            }

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

void PSModel::CalcLinearCombinationInternal(int *left, int right, vec3 &result)
{
    static const int SampleSize = 6;

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

void PSModel::InitGrayScaleWeight()
{
    for (int i = 0; i < 256; i++)
    {
        GrayScaleWeight[i] = 1 + 3 * std::cos((i) / 510.0 * PI);
    }
}

void PSModel::LoadObjectPixelNormals_CUDA()
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
        ptr_result, ball_pixel_count, h, w, dimension, 20, Configuration.ShadowThreshold, 0);
    else if (Configuration.PixelDistance == ModelConfiguration::PixelDistanceMethod::Angle_Method)
        CudaGetNearestPixelIndex(ptr_ball_pixel, ptr_object_pixel, ptr_object_shadow, ptr_gray_scale_weight, 
        ptr_result, ball_pixel_count, h, w, dimension, 20, Configuration.ShadowThreshold, 2);
    else if (Configuration.PixelDistance == ModelConfiguration::PixelDistanceMethod::Euclidiean_Method)
        CudaGetNearestPixelIndex(ptr_ball_pixel, ptr_object_pixel, ptr_object_shadow, ptr_gray_scale_weight, 
        ptr_result, ball_pixel_count, h, w, dimension, 20, Configuration.ShadowThreshold, 1);

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

void PSModel::LoadObjectPixelNormals_CPU_Shadow()
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

void PSModel::LoadBallPixelsInternal(const CircleType &circle, PixelInfoSet &pixels_p)
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
            //p1.Position = vec3(R - w, R - h, z);
            p1.SetImageData(&ImageData);
            pixels->push_back(p1);

            PixelInfo p2;
            p2.Index.X = (X - w);
            p2.Index.Y = (Y + h);
            p2.Normal = uvec3(-w, h, z);
            //p2.Position = vec3(R - w, R + h, z);
            p2.SetImageData(&ImageData);
            pixels->push_back(p2);

            PixelInfo p3;
            p3.Index.X = (X + w);
            p3.Index.Y = (Y + h);
            p3.Normal = uvec3(w, h, z);
            //p3.Position = vec3(R + w, R + h, z);
            p3.SetImageData(&ImageData);
            pixels->push_back(p3);

            PixelInfo p4;
            p4.Index.X = (X + w);
            p4.Index.Y = (Y - h);
            p4.Normal = uvec3(w, -h, z);
            //p4.Position = vec3(R + w, R - h, z);
            p4.SetImageData(&ImageData);
            pixels->push_back(p4);
        }
    }
    pixels_p.SetData(pixels);
}

void PSModel::Initialize()
{
    InitGrayScaleWeight();
}

void PSModel::SampleLightingVectors(double *samples, int *indices, int sample_size)
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

void PSModel::GetMiddleThreeIndex(NormalType *data, int dimension, int *ii)
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


   /* ScaleWithIndex<NormalType> *data_i = new ScaleWithIndex<NormalType>[dimension];
    for (int i = 0; i < dimension; ++i)
    {
        data_i[i].v = data[i];
        data_i[i].i = i;
    }
    partial_sort(data_i, data_i + dimension, data_i + dimension, [](const ScaleWithIndex<NormalType> &i1, const ScaleWithIndex<NormalType> &i2) { return i1.v > i2.v; });
    ii[0] = data_i[dimension / 2].i;
    ii[1] = data_i[dimension / 2 + 1].i;
    ii[2] = data_i[dimension / 2 + 2].i;
    delete [] data_i;*/

    sort(ii, ii + 3);

}

void PSModel::CalcLinearCombination()
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

void PSModel::CreateShadowRemovedImages()
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

void PSModel::SmoothObjectNormalField(int count, bool only_border)
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
        SmoothObjectNormalFieldInternal(ImageData.ObjectPixels, Configuration.ObjectLoadingRegion.width(), Configuration.ObjectLoadingRegion.height(), only_border);
    }
}

void PSModel::LoadObjectDepth()
{
    LoadObjectDepthInternal(ImageData.ObjectPixels, Configuration.ObjectLoadingRegion);

    // configuration
    // Nothing to do

    // progress
    // TODO
}

void PSModel::LoadObjectPixelNormals()
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

void PSModel::LoadTargetBallPixels(CircleType circle)
{
    LoadBallPixelsInternal(circle, ImageData.TargetObjectPixels);
    // progress
    // TODO
}

void PSModel::LoadBallPixels()
{
    LoadBallPixelsInternal(Configuration.ObjectLoadingCircle, ImageData.BallPixels);

    // progress
    // TODO
}


PSModel * PSModel::CreateModel(const string &dir)
{
    auto *retval = new PSModel();
    retval->Configuration.ImageSetSource = dir;
    retval->ImageData.NormalCube = ImageDataCubeFactory::CreateNormalCubeFromFiles(dir);
    retval->Configuration.ImageCount = retval->ImageData.NormalCube->GetC();
    return retval;
}

PSModel::PSModel()
{
    Initialize();
}

PixelInfoSet PSModel::RectanglizePixelInfoSet(PixelInfoSet pixels, QRect region)
{
    int W = region.width();
    int H = region.height();
    vector<PixelInfo> *retval_pointer = new vector<PixelInfo>();
    retval_pointer->reserve(W * H);
    auto &pixel_array = *pixels;
    for (int h = 0; h < H; ++h)
    {
        for (int w = 0; w < W; ++w)
        {
            PixelInfo p1;
            p1.Index.X = (region.x() + w);
            p1.Index.Y = (region.y() + h);
            p1.Position.X = w;
            p1.Position.Y = h;
            p1.SetImageData(&ImageData);

            bool found = false;
            for each (const PixelInfo &pixel in pixel_array)
            {
                if (pixel.Index == p1.Index)
                {
                    found = true;
                    p1.Normal = pixel.Normal;
                    p1.SetDarkPixel(false);
                    break;
                }
            }

            if (!found)
            {
                p1.Normal = uvec3(0.0, 0.0, 1.0);
                p1.SetDarkPixel(true);
            }
            retval_pointer->push_back(p1);
        }
    }

    PixelInfoSet retval;
    retval.SetData(retval_pointer);
    LoadObjectDepthInternal(retval, region);
    return retval;
}

PixelInfoSet PSModel::GetGroundTruth()
{
    ImageData.GroundTruth = RectanglizePixelInfoSet(ImageData.TargetObjectPixels, Configuration.ObjectLoadingRegion);
    return ImageData.GroundTruth;
}

void PSModel::LoadActuralNormal(PixelInfoSet pixels, QRect region)
{
    int w = region.width();
    int h = region.height();
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            vec3 r;
            vec3 p = pixels[j * w + i].Position;
            if (i > 0 && j > 0)
            {
                vec3 n = (pixels[(j - 1) * w + i].Position - p) ^ (pixels[j * w + i - 1].Position - p);
                r += n / n.GetLength();
            }
            if (i > 0 && j < h - 1)
            {
                vec3 n = (pixels[j * w + i - 1].Position - p) ^ (pixels[(j + 1) * w + i].Position - p);
                r += n / n.GetLength();
            }
            if (i < w - 1 && j < h - 1)
            {
                vec3 n = (pixels[(j + 1) * w + i].Position - p) ^ (pixels[j * w + i + 1].Position - p);
                r += n / n.GetLength();
            }
            if (i < w - 1 && j > 0)
            {
                vec3 n = (pixels[j * w + i + 1].Position - p) ^ (pixels[(j - 1) * w + i].Position - p);
                r += n / n.GetLength();
            }
            // here, use the negative flag '-', because the y-axis of the object space is from up to down of the screen.
            pixels[j * w + i].ActualNormal = -uvec3(r.X, r.Y, r.Z);
        }
    }
}

pair<QImage, PixelInfoSet> PSModel::GetReconstructDifference()
{
    QRect region = Configuration.ObjectLoadingRegion;
    int width = region.width();
    int height = region.height();
    vector<PixelInfo> *pixels = new vector<PixelInfo>();
    pixels->reserve(width * height);
    int size = ImageData.ObjectPixels->size();
    QImage bmp(width, height, QImage::Format_RGB888);
    for (int i = 0; i < size; ++i)
    {
        PixelInfo &pixel1 = ImageData.ObjectPixels[i];
        PixelInfo &pixel2 = ImageData.GroundTruth[i];
        PixelInfo pixel;
        pixel.GetIndex() = pixel1.Index;
        pixel.Normal = uvec3(0.0, 0.0, 1.0);
        pixel.SetImageData(&ImageData);
        pixel.Position.X = pixel1.Position.X;
        pixel.Position.Y = pixel1.Position.Y;
        pixel.Position.Z = std::abs(pixel1.Position.Z - pixel2.Position.Z);

        pixels->push_back(pixel);

        double angle = 0;
        if (!pixel1.DarkPixel() && !pixel2.DarkPixel())
        {
            double dot = pixel1.Normal * pixel2.Normal;
            dot = min(1.0, max(0.0, dot));
            angle = acos(dot);
        }
        QColor color = GetAngleErrorColor(angle / PI * 180);
        bmp.setPixel(pixel1.Position.X, pixel2.Position.Y, color.rgb());
    }

    PixelInfoSet retval;
    retval.SetData(pixels);

    LoadActuralNormal(retval, region);


    return std::make_pair(bmp, retval);

}



namespace
{
    QColor GetColor(QColor c1, QColor c2, double ratio);
    QColor GetColor(double ratio)
    {
        QColor colors[] = { QColor(0,0,180), QColor(0,255,255), 
            QColor(100,255,0), QColor(255,255,0), QColor(255,0, 0), QColor(128,0,0)};

        if (ratio <= 0.2)
            return GetColor(colors[0], colors[1], ratio / 0.2);
        else if (ratio <= 0.4)
            return GetColor(colors[1], colors[2], (ratio - 0.2) / 0.2);
        else if (ratio <= 0.6)
            return GetColor(colors[2], colors[3], (ratio - 0.4) / 0.2);
        else if (ratio <= 0.8)
            return GetColor(colors[3], colors[4], (ratio - 0.6) / 0.2);
        else 
            return GetColor(colors[4], colors[5], (ratio - 0.8) / 0.2);
    }

    QColor GetColor(QColor c1, QColor c2, double ratio)
    {
        ratio = max(0, min(1, ratio));
        double r, g, b;
        r = c1.red() + (c2.red() - c1.red()) * ratio;
        g = c1.green() + (c2.green() - c1.green()) * ratio;
        b = c1.blue() + (c2.blue() - c1.blue()) * ratio;
        r = max(0, min(255, r));
        g = max(0, min(255, g));
        b = max(0, min(255, b));
        return QColor((int)r, (int)g, (int)b);
    }
}
QColor PSModel::GetAngleErrorColor( double angle )
{
    return GetColor(angle / MaxAngleErrorForColorIndex);
}

