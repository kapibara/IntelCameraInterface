#include "intelvideosource.h"

#include <iostream>

IntelVideoSource::IntelVideoSource(DepthProfile profileType)
{

    PXCImage::ImageInfo dprofile ={ 320, //width
                                    240, //height
                                    PXCImage::COLOR_FORMAT_DEPTH};

    if(profileType!=IntelVideoSource::PROFILE_DEPTH){
        dprofile.format = PXCImage::COLOR_FORMAT_VERTICES;
    }

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
    case IntelVideoSource::IMAGE_CONFIDENCE:
    case IntelVideoSource::IMAGE_UV:
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
    case IntelVideoSource::IMAGE_CONFIDENCE:
    case IntelVideoSource::IMAGE_UV:
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


    //sts_ = camera->SetProperty(PXCCapture::Device::PROPERTY_DEPTH_SMOOTHING,true);
    //if (!isOk()){
    //    throw IntelCameraException(sts_,"Could not turn off depth smoothing");
    //}

    pxcF32 toReturn;
    sts_ = camera->QueryProperty(PXCCapture::Device::PROPERTY_DEPTH_LOW_CONFIDENCE_VALUE,&toReturn);
    if (isOk()){
        std::cout << "Confidence value: " << (float)toReturn << std::endl;
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

        /*get calibration parameters and print them*/
        PXCPointF32 calib;
        sts_ = camera->QueryPropertyAsPoint(PXCCapture::Device::PROPERTY_DEPTH_FOCAL_LENGTH,&calib);
        if(isOk()){
            std::cout << "depth focal length: (" << calib.x << ";" << calib.y << ")" << std::endl;
            calib_.depth_fx = calib.x;
            calib_.depth_fy = calib.y;
        }

        sts_ = camera->QueryPropertyAsPoint(PXCCapture::Device::PROPERTY_DEPTH_PRINCIPAL_POINT,&calib);
        if(isOk()){
            std::cout << "depth principal point: (" << calib.x << ";" << calib.y << ")" << std::endl;
            calib_.depth_cx = calib.x;
            calib_.depth_cy = calib.y;
        }

        sts_ = camera->QueryPropertyAsPoint(PXCCapture::Device::PROPERTY_COLOR_FOCAL_LENGTH,&calib);
        if(isOk()){
            std::cout << "color focal length: (" << calib.x << ";" << calib.y << ")" << std::endl;
            calib_.color_fx = calib.x;
            calib_.color_fy = calib.y;
        }

        sts_ = camera->QueryPropertyAsPoint(PXCCapture::Device::PROPERTY_COLOR_PRINCIPAL_POINT,&calib);
        if(isOk()){
            std::cout << "color principal point: (" << calib.x << ";" << calib.y << ")" << std::endl;
            calib_.color_cx = calib.x;
            calib_.color_cy = calib.y;
        }

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

/*this is for "real" images*/
int IntelVideoSource::sizeInBytes(PXCImage::ColorFormat format){
    switch(format){
    case PXCImage::COLOR_FORMAT_DEPTH:
        return sizeof(short);
    case PXCImage::COLOR_FORMAT_RGB24:
        return 3;
    case PXCImage::COLOR_FORMAT_RGB32:
        return sizeof(int);
    case PXCImage::COLOR_FORMAT_VERTICES:
        return 3*sizeof(int);
    default:
        return -1;
    }
}

/*this is for all possible buffer types*/
int IntelVideoSource::sizeInBytes(ImageType type){
    switch(type){
    //16bit integer
    case IntelVideoSource::IMAGE_DEPTH:
    case IntelVideoSource::IMAGE_CONFIDENCE:
        return sizeInBytes(curDProfile_.format);
        break;
    case IntelVideoSource::IMAGE_RGB:
        return sizeInBytes(curCProfile_.format);
        break;
    //32bit float
    case IntelVideoSource::IMAGE_UV:
        return sizeof(float);
        break;
    case IntelVideoSource::IMAGE_VERTICES:
        if (curDProfile_.format!=PXCImage::COLOR_FORMAT_VERTICES){
            return -1;
        }
        return sizeInBytes(curDProfile_.format);
        break;
    default:
        return -1;
    }
}

int IntelVideoSource::expectedBufferSize(ImageType type){
    switch(type){
    case IntelVideoSource::IMAGE_RGB:
        return curCProfile_.width*curCProfile_.height*sizeInBytes(IntelVideoSource::IMAGE_RGB);
        break;
    case IntelVideoSource::IMAGE_DEPTH:
        return curDProfile_.width*curDProfile_.height*sizeInBytes(IntelVideoSource::IMAGE_DEPTH);
        break;
    case IntelVideoSource::IMAGE_CONFIDENCE:
        return curDProfile_.width*curDProfile_.height*sizeInBytes(IntelVideoSource::IMAGE_CONFIDENCE);
        break;
    case IntelVideoSource::IMAGE_UV:
        //2 coordinates (x,y)
        return 2*curDProfile_.width*curDProfile_.height*sizeInBytes(IntelVideoSource::IMAGE_UV);
        break;
     case IntelVideoSource::IMAGE_VERTICES:
        return curDProfile_.width*curDProfile_.height*sizeInBytes(IntelVideoSource::IMAGE_VERTICES);
        break;
    default:
        return -1;
    }
}

void IntelVideoSource::copyToBuffer(const PXCImage::ImageData &data, ImageType type, unsigned char *buffer)
{
    int copyLength = expectedBufferSize(type);
    switch(type){
    case IntelVideoSource::IMAGE_RGB:
    case IntelVideoSource::IMAGE_DEPTH:
        memcpy(buffer,data.planes[0],copyLength);
        break;
    case IntelVideoSource::IMAGE_CONFIDENCE:
        memcpy(buffer,data.planes[1],copyLength);
        break;
    case IntelVideoSource::IMAGE_UV:
        memcpy(buffer,data.planes[2],copyLength);
        break;
    case IntelVideoSource::IMAGE_VERTICES:
        memcpy(buffer,data.planes[0],copyLength);
        break;
    default:
        ;
    };
}

bool IntelVideoSource::retrieve(unsigned char *buffer[], ImageType type, int timeout){
    PXCSmartPtr<PXCImage> image;
    PXCImage::ImageData data;

    switch(type){
    case IntelVideoSource::IMAGE_RGB:
        sts_ = RGBStream_->ReadStreamAsync(&image,&sps_);
        if (!isOk()) throw IntelCameraException(sts_,"Could not execute async read on the color stream");

        break;
    case IntelVideoSource::IMAGE_DEPTH:
    case IntelVideoSource::IMAGE_CONFIDENCE:
    case IntelVideoSource::IMAGE_UV:
        sts_ = DStream_->ReadStreamAsync(&image,&sps_);
        if (!isOk()) throw IntelCameraException(sts_,"Could not execute async read on the depth stream");

        break;
    default:
        return false;
    };



    sts_ = sps_->Synchronize(timeout);
    if (!isOk()) throw IntelCameraException(sts_,"Timeout waiting for data from RGB stream");

    sts_ = image->AcquireAccess(PXCImage::ACCESS_READ,&data);
    if (!isOk()) throw IntelCameraException(sts_,"Error getting access to the RGB image");

    if (type == IntelVideoSource::IMAGE_DEPTH){
        copyToBuffer(data,IntelVideoSource::IMAGE_DEPTH,buffer[0]);
        copyToBuffer(data,IntelVideoSource::IMAGE_CONFIDENCE,buffer[1]);
        copyToBuffer(data,IntelVideoSource::IMAGE_UV,buffer[2]);
    }else{
        copyToBuffer(data,type,buffer[0]);
    }

    image->ReleaseAccess(&data);

    image.ReleaseRef();
    sps_.ReleaseRef();

    return true;
}
