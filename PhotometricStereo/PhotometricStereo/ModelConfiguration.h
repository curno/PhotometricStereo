#pragma once

#include "stdafx.h"
#include "Math.hpp"

typedef Vector3<int> CircleType;

class ModelConfiguration
{
public:
    enum class ShadowDetectionMethod { Ignore_Method, ThreeNoShadow_Method, PerPixel_Method, /* count */ ShadowDetectionMethod_Count };
    enum class PixelDistanceMethod { Manhaton_Method, Euclidiean_Method, Angle_Method, /* count */ PixelDistanceMethod_Count };
    enum class ShadowUseMethod { Ignore_Shadow, Remove_Shadow, /* count */ ShadowUse_Count };
    
    string ImageSetSource;
    bool UsingCuda;
    CircleType ShadowDetectionCircle;
    ShadowDetectionMethod ShadowDetection;
    CircleType ObjectLoadingCircle;
    QRect ObjectLoadingRegion;
    PixelDistanceMethod PixelDistance;
    ShadowUseMethod ShadowUse;
    int NormalSmoothing;
    int BorderNormalSmoothing;
    int ShadowThreshold;
    int ImageCount;
    bool UseMiddleThree;

    int ElapsedMiliSeconds;
    ModelConfiguration() : UsingCuda(false), ShadowDetection(ShadowDetectionMethod::Ignore_Method), PixelDistance(PixelDistanceMethod::Manhaton_Method), NormalSmoothing(0), BorderNormalSmoothing(0), ShadowThreshold(20), ElapsedMiliSeconds(0), UseMiddleThree(false) { }

    string readonly(Description);
    string GetDescription() const
    {
        ostringstream oss;
        string indent = "    ";
        string endl = "<br/>";
        string hline = "<hr/>";
        string color = "rgb(255, 201, 14)";
        oss << "Image Set: " << ImageSetSource << endl;
        oss << "Image Count:" << ImageCount << endl;
        oss << "Shadow Detection: " << "<span style=\"color:" << color <<";\">" << ShadowDetetionMethodStr(ShadowDetection) << "</span>" << endl;
        oss << "Using Cuda: " << "<span style=\"color:" << color <<";\">" << (UsingCuda ? "Yes" : "No") << "</span>" << endl;
        if (ShadowDetection != ShadowDetectionMethod::Ignore_Method)
        {
            oss << indent << "Detection Circle: " << "(" << ShadowDetectionCircle.X << ", " << ShadowDetectionCircle.Y << ") " << ShadowDetectionCircle.Z << endl; 
            oss << indent << "Shadow Threshold: " << ShadowThreshold << endl;
        }
        if (!ObjectLoadingRegion.isNull())
        {
            oss << "Object Region: " << ObjectLoadingRegion.x() << ", " << ObjectLoadingRegion.y() 
                << ", " << ObjectLoadingRegion.width() << ", " << ObjectLoadingRegion.height() << endl;
            oss << "Circle Region: " << "(" << ObjectLoadingCircle.X << ", " << ObjectLoadingCircle.Y << ") " << ObjectLoadingCircle.Z << endl;
            oss << "Shadow Use: "<< "<span style=\"color:" << color <<";\">" << ShadowUseStr(ShadowUse) << "</span>" << endl;
            oss << "Pixel Distance: "<< "<span style=\"color:" << color <<";\">" << PixelDistanceMethodStr(PixelDistance) << "</span>" << endl;
            oss << "Normal Smooth Count: " << NormalSmoothing << endl;
            oss << "Border Normal Smooth Count: " << BorderNormalSmoothing << endl;
        }
        if (ElapsedMiliSeconds != 0)
        {
            oss << "Object Pixel:" << ObjectLoadingRegion.width() * ObjectLoadingRegion.height() << endl;
            oss << "Calculation Amount:" << ObjectLoadingRegion.width() * ObjectLoadingRegion.height() * ObjectLoadingCircle.Z * ObjectLoadingCircle.Z * PI * ImageCount << endl;
            oss << "Elapsed Time: " << "<span style=\"color:" << color << ";\">" << ElapsedMiliSeconds << " ms" << "</span>"  << endl;
        }
        return oss.str();
    }

    static string ShadowDetetionMethodStr(ShadowDetectionMethod method)
    {
        switch (method)
        {
        case ShadowDetectionMethod::Ignore_Method:
            return "Ignore";
        case ShadowDetectionMethod::PerPixel_Method:
            return "Per pixel";
        case ShadowDetectionMethod::ThreeNoShadow_Method:
            return "Three no shadow";
        default:
            return "";
        }
    }
    static string PixelDistanceMethodStr(PixelDistanceMethod method)
    {
        switch (method)
        {
        case PixelDistanceMethod::Manhaton_Method:
            return "Manhaton";
        case PixelDistanceMethod::Angle_Method:
            return "Angle";
        case PixelDistanceMethod::Euclidiean_Method:
            return "Euclidiean";
        default:
            return "";
        }
    }

    static string ShadowUseStr(ShadowUseMethod method)
    {
        switch (method)
        {
        case ShadowUseMethod::Ignore_Shadow:
            return "Ignore Shadow Pixel";
        case ShadowUseMethod::Remove_Shadow:
            return "Remove Shadow";
        default:
            return "";
        }
    }
};