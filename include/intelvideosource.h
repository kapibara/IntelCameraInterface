#ifndef INTELVIDEOSOURCE_H
#define INTELVIDEOSOURCE_H

#include "pxcsession.h"
#include "pxcsmartptr.h"
#include "pxccapture.h"
#include "pxcscheduler.h"

#include <string>
#include <ostream>

/*exception class with all possible errors*/
class IntelCameraException{

    friend std::ostream& operator<<(std::ostream& out, const IntelCameraException& exception){
        out << "Error code: " << exception.errorCode_ << std::endl << exception.message_ << std::endl;

        return out;
    }
public:
    IntelCameraException(pxcStatus errorCode,const std::string &message = ""): errorCode_(errorCode),message_(message) {

    }

private:
    std::string message_;
    pxcStatus errorCode_;
};


struct IntelCameraCalibration{
    float color_fx;
    float color_fy;
    float color_cx;
    float color_cy;

    float depth_fx;
    float depth_fy;
    float depth_cx;
    float depth_cy;
};

/*wrapper class for Intel camera*/

class IntelVideoSource
{


public:
    enum ImageType{
        IMAGE_RGB = 0x01,
        IMAGE_DEPTH = 0x02,
        IMAGE_CONFIDENCE = 0x03,
        IMAGE_UV = 0x04,
        IMAGE_VERTICES = 0x05
    };

    enum DepthProfile{
        PROFILE_DEPTH = 0x01,
        PROFILE_VERTICES = 0x02
    };


    IntelVideoSource(DepthProfile profileType = IntelVideoSource::PROFILE_DEPTH);
    IntelVideoSource(const PXCImage::ImageInfo &DProfile,const PXCImage::ImageInfo &CProfile);
    ~IntelVideoSource();

    bool open();
    bool isOpened();
    bool close();

    int width(ImageType type);
    int height(ImageType type);

    IntelCameraCalibration getCalibration();

    /*that is blocking call!*/
    bool retrieve(unsigned char *buffer[], ImageType imgType, int timeout=PXCScheduler::SyncPoint::TIMEOUT_INFINITE);

    int expectedBufferSize(ImageType type);

    int sizeInBytes(PXCImage::ColorFormat format);
    int sizeInBytes(ImageType type);

private:


    bool setProfile(PXCSmartPtr<PXCCapture::VideoStream> &stream,const PXCImage::ImageInfo  &info);
    bool isOk(){
        return (sts_ >= PXC_STATUS_NO_ERROR);
    }

    bool isOpened_;

    void copyToBuffer(const PXCImage::ImageData &data, ImageType type, unsigned char *buffer);

    PXCSmartPtr<PXCCapture::VideoStream> RGBStream_;
    PXCSmartPtr<PXCCapture::VideoStream> DStream_;

    PXCSmartPtr<PXCSession> session_;
    PXCSmartPtr<PXCScheduler::SyncPoint> sps_;

    PXCImage::ImageInfo curDProfile_,curCProfile_;

    pxcStatus sts_; // status of the last operation

    IntelCameraCalibration calib_;
};

#endif // INTELVIDEOSOURCE_H
