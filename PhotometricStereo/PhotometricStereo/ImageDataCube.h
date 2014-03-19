#pragma once 

#include "stdafx.h"

#include "PixelInfo.h"

class ImageDataCubeFactory;

struct ImageDataCubeBase abstract
{
    friend class ImageDataCubeFactory;
    int readonly(M);
    int GetM() const { return M_; }
    int readonly(N);
    int GetN() const { return N_; }
    int readonly(C);
    int GetC() const { return C_; }
    ImageDataCubeBase(int M, int N, int C) : M_(M), N_(N), C_(C), QImageCache_(C, nullptr), CvImageCache_(C, nullptr), CvImageForQImage_(C, nullptr), Cube_(nullptr) { }
    virtual ~ImageDataCubeBase()
    {
        for (auto i = QImageCache_.begin(); i != QImageCache_.end(); ++i)
            delete *i;
        for (auto i = CvImageCache_.begin(); i != CvImageCache_.end(); ++i)
            cvReleaseImage(&(*i));
        for (auto i = CvImageForQImage_.begin(); i != CvImageForQImage_.end(); ++i)
            cvReleaseImage(&(*i));
        delete []Cube_;
    }
    
    IplImage *GetCvImage(int index)
    {
        assert(CvImageCache_[index] != nullptr);
        return CvImageCache_[index];
    }
    QImage *GetQImage(int index) 
    {
        if (QImageCache_[index] == nullptr)
        {
            IplImage *cv_image = GetCvImage(index);
            IplImage *image = cvCreateImage(cvSize(cv_image->width, cv_image->height), IPL_DEPTH_8U, cv_image->nChannels);
            cvConvertScaleAbs(cv_image, image);
            QImage *q_image = new QImage((uchar *)image->imageData, image->width, image->height, image->widthStep, QImage::Format_Indexed8);
            QVector<QRgb> my_table;
            for(int i = 0; i < 256; i++) 
                my_table.push_back(qRgb(i, i, i));
            q_image->setColorTable(my_table);
            if (false)
                q_image->save("D:\\1.bmp");
            QImageCache_[index] = q_image;
            CvImageForQImage_[index] = image;
        }
        return QImageCache_[index];
    }

    void SaveToFolder(const QString &dir_name)
    {
        QDir dir(dir_name);
        for (int i = 0; i < C; ++i)
            GetQImage(i)->save(dir.filePath(QString::number(i) + ".bmp"));
    }
protected:
    vector<QImage *> QImageCache_;
    vector<IplImage *> CvImageForQImage_;
    vector<IplImage *> CvImageCache_;
    int M_, N_, C_;
    void *Cube_;
};
template<typename DataType>
class ImageDataCube : public ImageDataCubeBase
{
public:

    ImageDataCube(int M, int N, int C) : ImageDataCubeBase(M, N, C) { }

    DataType *GetData(int i, int j) { return static_cast<DataType *>(Cube_) + ((i * M + j) * C); }
    const DataType *GetData(int i, int j) const { return static_cast<DataType *>(Cube_) + ((i * M + j) * C); }
};

class ImageDataCubeFactory
{
public:
    template<typename CubeType>
    static shared_ptr<ImageDataCube<CubeType>> CreateNormalCubeFromCvImages(vector<IplImage *> &images)
    {
        if (images.empty())
            return nullptr;
        int m = images[0]->width;
        int n = images[0]->height;
        int c = images.size();
        ImageDataCube<CubeType> *retval = new ImageDataCube<CubeType>(m, n, c);
        retval->CvImageCache_.assign(images.begin(), images.end());
        retval->Cube_ = new CubeType[m * n * c];
        CubeType *current = static_cast<CubeType *>(retval->Cube_);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < m; ++j)
                for (int k = 0; k < c; ++k)
                    *(current++) = static_cast<CubeType>(GetGrayScaleAt(retval->CvImageCache_[k], j, i));
        return std::shared_ptr<ImageDataCube<CubeType>>(retval);
    }

    static shared_ptr<ImageDataCube<ImageDataCubeType<NormalCube>::Type>> CreateNormalCubeFromFiles(const string &dir_name)
    {
        vector<IplImage *> images;
        QDir dir(FromStdStringToQString(dir_name));
        QFileInfoList list = dir.entryInfoList();
        foreach (QFileInfo info, list)
        {
            string s = FromQStringToStdString(info.absoluteFilePath());
            IplImage *image = cvLoadImage(s.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
            if (image != nullptr)
                images.push_back(image);
        }

        return CreateNormalCubeFromCvImages<ImageDataCubeType<NormalCube>::Type>(images);

    }

    static double GetGrayScaleAt(IplImage *image, int x, int y)
    {
        switch (image->depth)
        {
        case IPL_DEPTH_8U:
            return CV_IMAGE_ELEM(image, unsigned char, y, x);
        case IPL_DEPTH_16S:
            return CV_IMAGE_ELEM(image, short, y, x);
        case IPL_DEPTH_64F:
            return CV_IMAGE_ELEM(image, double, y, x);
        case IPL_DEPTH_32F:
            return CV_IMAGE_ELEM(image, float, y, x);
        case IPL_DEPTH_32S:
            return CV_IMAGE_ELEM(image, int, y, x);
        default:
            return DBL_MAX;
        }
    }

    template<typename T>
    static void SetGrayScaleAt(IplImage *image, int x, int y, T v)
    {
        switch (image->depth)
        {
        case IPL_DEPTH_8U:
            (reinterpret_cast<unsigned char *>(image->imageData))[y * image->widthStep + x] = v;
            return;
        default:
            return;
        }
    }
   
};

template<typename DataType>
DataType *PixelInfo::GetData(CubeType cube_index)
{
    return ((ImageDataCube<DataType> *)(this->ImageModel_->GetImageDataCube(cube_index).get()))->GetData(this->Index.Y, this->Index.X);
}

template<typename DataType>
const DataType *PixelInfo::GetData(CubeType cube_index) const
{
    return ((const ImageDataCube<DataType> *)(this->ImageModel_->GetImageDataCube(cube_index).get()))->GetData(this->Index.Y, this->Index.X);
}
