#include "intelvideosource.h"

IntelVideoSource::IntelVideoSource()
{
    PXCImage::ImageInfo dprofile ={ 320, //width
                                    240, //height
                                    PXCImage::COLOR_FORMAT_DEPTH};

    PXCImage::ImageInfo cprofile = { 640, //width
                                     480, //height
                                     PXCImage::COLOR_FORMAT_RGB24}; //color format

    isOpened_ = false;
    curDProfile_ = dprofile;
    curCProfile_ = cprofile;

    open();
}

IntelVideoSource::IntelVideoSource(const PXCImage::ImageInfo &DProfile, //color format
                                   const PXCImage::ImageInfo &RGBProfile) //color format
{

    isOpened_ = false;
    curDProfile_ = DProfile;
    curCProfile_ = RGBProfile;

    open();
}

IntelVideoSource::~IntelVideoSource()
{
    close();
}

int IntelVideoSource::width(ImageType type)
{
    switch(type)
    {
    case IntelVideoSource::IMAGE_RGB:
       return curCProfile_.width;
    case IntelVideoSource::IMAGE_DEPTH:
        return curDProfile_.width;
    default:
        return -1;
    }
}

int IntelVideoSource::height(ImageType type)
{
    switch(type)
    {
    case IntelVideoSource::IMAGE_RGB:
        return curCProfile_.height;
    case IntelVideoSource::IMAGE_DEPTH:
        return curDProfile_.height;
    default:
        return -1;
    }

}

bool IntelVideoSource::open(){

    if (isOpened_){
        return false;
    }

    sts_ = PXCSession_Create(&session_);
    if (!isOk()){
        throw IntelCameraException(sts_,"Could not create session");
    }

    PXCSmartPtr<PXCCapture> capture;
    sts_ = session_->CreateImpl<PXCCapture>(&capture);
    if (!isOk()){
        throw IntelCameraException(sts_,"Could not create capture");
    }

    PXCSmartPtr<PXCCapture::Device> camera;
    sts_ = capture->CreateDevice(0,&camera);
    if (!isOk()){
        throw IntelCameraException(sts_,"Could not retrieve the first device");
    }

    PXCCapture::Device::StreamInfo sinfo;
    int depthStreamIdx=-1,colorStreamIdx=-1;

    for(int i=0; colorStreamIdx<0 || depthStreamIdx<0; i++){
        sts_ = camera->QueryStream(i,&sinfo);
        if (isOk()) {
            if (sinfo.imageType == PXCImage::IMAGE_TYPE_COLOR && colorStreamIdx<0){
                colorStreamIdx = i;
            }
            if (sinfo.imageType == PXCImage::IMAGE_TYPE_DEPTH && depthStreamIdx<0){
                depthStreamIdx = i;
            }
        }else{
            throw IntelCameraException(sts_,"Error while querying the device");
            break;
        }
    }

    if (colorStreamIdx >=0 && depthStreamIdx >=0){
        sts_ = camera->CreateStream(colorStreamIdx,PXCCapture::VideoStream::CUID,(void **)&RGBStream_);
        if (!isOk()){
            throw IntelCameraException(sts_,"Could not create color stream.");
        }
        sts_ = camera->CreateStream(depthStreamIdx,PXCCapture::VideoStream::CUID,(void **)&DStream_);
        if(!isOk()){
            throw IntelCameraException(sts_,"Could not create depth stream.");
        }

        if (!setProfile(RGBStream_,curCProfile_))
            throw IntelCameraException((pxcStatus)-1,"Could not set RGB profile");

        if (!setProfile(DStream_,curDProfile_))
            throw IntelCameraException((pxcStatus)-1,"Could not set depth profile");

    }
    else{
        throw IntelCameraException((pxcStatus)-1,"Could not locate color or depth stream");
    }

    isOpened_ = true;

    return true;
}

bool IntelVideoSource::isOpened(){
    return isOpened_;
}

bool IntelVideoSource::close(){
    RGBStream_.ReleaseRef();
    DStream_.ReleaseRef();

    return true;
}

bool IntelVideoSource::setProfile(PXCSmartPtr<PXCCapture::VideoStream> &stream, const PXCImage::ImageInfo &info)
{
    PXCCapture::VideoStream::ProfileInfo pinfo;
    bool isSet = false;

    for(int i=0; ;i++){
        sts_ = stream->QueryProfile(i,&pinfo);
        if (!isOk()) break;

        if (pinfo.imageInfo.width == info.width &&
            pinfo.imageInfo.height == info.height &&
            pinfo.imageInfo.format == info.format){
            sts_ = stream->SetProfile(&pinfo);
            if (isOk()){
                isSet = true;
                break;
            }else{
                throw IntelCameraException(sts_,"Error while setting stream profile");
            }
        }
    }

    return isSet;
}

int IntelVideoSource::sizeInBytes(PXCImage::ColorFormat format){
    switch(format){
    case PXCImage::COLOR_FORMAT_DEPTH:
        return 2;
    case PXCImage::COLOR_FORMAT_RGB24:
        return 3;
    case PXCImage::COLOR_FORMAT_RGB32:
        return 4;
    default:
        return -1;
    }
}

int IntelVideoSource::sizeInBytes(ImageType type){
    switch(type){
    case IMAGE_DEPTH:
        return sizeInBytes(curDProfile_.format);
    case IMAGE_RGB:
        return sizeInBytes(curCProfile_.format);
    default:
        return -1;
    }
}

int IntelVideoSource::expectedBufferSize(ImageType type){
    switch(type){
    case IntelVideoSource::IMAGE_RGB:
        return curCProfile_.width*curCProfile_.height*sizeInBytes(curCProfile_.format);
        break;
    case IntelVideoSource::IMAGE_DEPTH:
        return curDProfile_.width*curDProfile_.height*sizeInBytes(curDProfile_.format);
        break;
    default:
        return -1;
    }
}

void IntelVideoSource::copyToRGB24(const PXCImage::ImageData &data, int imwidth, int imheight, unsigned char *buffer)
{
    int copyLength = expectedBufferSize(IMAGE_RGB);
    memcpy(buffer,data.planes[0],copyLength);
}

void IntelVideoSource::copyToDepth(const PXCImage::ImageData &data, int imwidth, int imheight, unsigned char *buffer)
{
    int copyLength = expectedBufferSize(IMAGE_DEPTH);
    memcpy(buffer,data.planes[0],copyLength);
}

bool IntelVideoSource::retrieve(unsigned char *buffer, ImageType type, int timeout){
    PXCSmartPtr<PXCImage> image;
    PXCImage::ImageData data;

    switch(type){
    case IntelVideoSource::IMAGE_RGB:
        sts_ = RGBStream_->ReadStreamAsync(&image,&sps_);
        if (!isOk()) throw IntelCameraException(sts_,"Could not execute async read on the color stream");

        sts_ = sps_->Synchronize(timeout);
        if (!isOk()) throw IntelCameraException(sts_,"Timeout waiting for data from RGB stream");

        sts_ = image->AcquireAccess(PXCImage::ACCESS_READ,&data);
        if (!isOk()) throw IntelCameraException(sts_,"Error getting access to the RGB image");

        copyToRGB24(data,curCProfile_.width,curCProfile_.height,buffer);

        image->ReleaseAccess(&data);

        image.ReleaseRef();
        sps_.ReleaseRef();

        break;
    case IntelVideoSource::IMAGE_DEPTH:
        sts_ = DStream_->ReadStreamAsync(&image,&sps_);
        if (!isOk()) throw IntelCameraException(sts_,"Could not execute async read on the depth stream");

        sts_ = sps_->Synchronize(timeout);
        if (!isOk()) throw IntelCameraException(sts_,"Timeout waiting for data from depth stream");

        sts_ = image->AcquireAccess(PXCImage::ACCESS_READ,&data);
        if (!isOk()) throw IntelCameraException(sts_,"Error getting access to the depth image");

        copyToDepth(data,curDProfile_.width,curDProfile_.height,buffer);

        image->ReleaseAccess(&data);

        image.ReleaseRef();
        sps_.ReleaseRef();

        break;
    default:
        return false;
    }

    return true;
}
