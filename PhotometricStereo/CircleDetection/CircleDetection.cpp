// CircleDetection.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "opencv.hpp"
#include <vector>
#include <string>

using namespace std;
void show(IplImage *image, char *name)
{
    cvNamedWindow(name);
    cvShowImage(name,image);
    cvWaitKey();
}

void showCircle( CvSeq * seq, IplImage * display, char *name);

int _tmain(int argc, _TCHAR* argv[])
{
    vector<IplImage *> images;
    vector<IplImage *> copys;
    for (int i = 0; i < 15; ++i)
    {
        char buffer[100]; 
        itoa(i, buffer, 10);
        IplImage *image = cvLoadImage((string("F:\\Research\\PhotometricStereo\\Data\\Real\\David\\selected\\") + buffer + ".JPG").c_str(), CV_LOAD_IMAGE_GRAYSCALE);
        images.push_back(image);
        IplImage *copy = cvCreateImage(cvSize(image->width, image->height), image->depth, image->nChannels);
        cvCopy(image, copy);
        cvSmooth(copy, copy, CV_GAUSSIAN, 3, 0.0, 0.0, 0.0);
        show(copy, buffer);
        cvSobel(copy, copy, 1, 1, 5);
        show(copy, buffer);
        copys.push_back(copy);
        if (i == 14)
        {
            auto storage = cvCreateMemStorage();
            auto *seq = cvHoughCircles(image, storage, CV_HOUGH_GRADIENT, 3, image->width / 2.0, 100, 100, 40, 100);
            showCircle(seq, image, "hehe");

        }
    }
    IplImage *img_sum = cvCreateImage(cvSize(images[0]->width, images[0]->height), IPL_DEPTH_32F, images[0]->nChannels);
    cvZero(img_sum);
    for each (IplImage *image in copys)
    {
        IplImage *img = cvCreateImage(cvSize(image->width, image->height), IPL_DEPTH_8U, image->nChannels);
        cvConvertScaleAbs(image, img);
        cvAdd(img, img_sum, img_sum);
        cvReleaseImage(&img);
    }
    IplImage *img_int = cvCreateImage(cvSize(images[0]->width, images[0]->height), IPL_DEPTH_8U, images[0]->nChannels);
    cvConvertScaleAbs(img_sum, img_int);
    IplImage *img_sum_ava = cvCreateImage(cvSize(images[0]->width, images[0]->height), IPL_DEPTH_8U, images[0]->nChannels);
    show(img_int, "a");
    cvConvertScale(img_sum, img_sum_ava, 1.0 / copys.size());
    show(img_sum_ava, "a");

    auto storage = cvCreateMemStorage();
    auto *seq = cvHoughCircles(img_sum_ava, storage, CV_HOUGH_GRADIENT, 2, img_int->width / 5.0, 100, 100, 40, 100);
    float *p = (float *)cvGetSeqElem(seq, 0);

    

    
    showCircle(seq, img_sum_ava, "a");
    showCircle(seq, images[1], "1");
    showCircle(seq, images[14], "0");
    
    
	return 0;
}

void showCircle( CvSeq * seq, IplImage * display, char *name)
{
    IplImage * display2 = cvCreateImage(cvSize(display->width,display->height),IPL_DEPTH_8U,3);
    cvCvtColor(display, display2, CV_GRAY2BGR);
    for(int i=0;i<seq->total;i++) //total在库函数中定义
    {
        float* p=(float*)cvGetSeqElem(seq,i);
        cvCircle(display2,cvPoint(cvRound(p[0]),cvRound(p[1])) ,3,CV_RGB(0,255,0),-1,8,0);  //绘制拟合圆的圆心，绿色
        cvCircle(display2,cvPoint(cvRound(p[0]),cvRound(p[1])) ,cvRound(p[2]),CV_RGB(255,0,0),1,8,0);		//绘制拟合圆，红色

    }
    show(display2, name);
}

