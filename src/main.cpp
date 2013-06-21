
#include <iostream>
#include <iomanip>

#include "pxcsession.h"
#include "pxcsmartptr.h"
#include "pxccapture.h"
#include "pxcscheduler.h"

#include "intelvideosource.h"

#include <opencv2/opencv.hpp>

using namespace std;

cv::Mat convertToOpenCVRGB(const PXCImage::ImageData &data, int imwidth, int imheight)
{
    //check the size and color format
    if (data.format == PXCImage::COLOR_FORMAT_RGB24){
        cv::Mat result(cv::Size(imwidth,imheight),CV_8UC3);
        uchar *ptr;
        int rows = result.rows, cols = result.cols, copyLength = data.pitches[0];

        if (result.isContinuous()){
            cout << "Matrix is continious" << endl;  //can i force this?
            cols *= rows;
            copyLength *= rows; //copy all bytes at once
            rows = 1;
        }

        for (int i=0; i<rows; i++){
            ptr = result.ptr(i);
            memcpy(ptr,&(data.planes[0][i*copyLength]),copyLength);
        }

        return result;
    }
    else{
        cout << "Image format is not RGB 24" << endl;

        return cv::Mat();
    }

}

cv::Mat convertToOpenCVDepth(const PXCImage::ImageData &data, int imwidth, int imheight){

    if (data.format == PXCImage::COLOR_FORMAT_DEPTH){
        cv::Mat result(cv::Size(imwidth,imheight),CV_16UC1);
        uchar *ptr;
        int rows = result.rows, cols = result.cols, copyLength = data.pitches[0];

        if (result.isContinuous()){
            cout << "Matrix is continious" << endl;  //can i force this?
            cols *= rows;
            copyLength *= rows; //copy all bytes at once
            rows = 1;
        }

        for (int i=0; i<rows; i++){
            ptr = result.ptr(i);
            memcpy(ptr,&(data.planes[0][i*copyLength]),copyLength);
        }

        return result;
    }
    else{
        cout << "Image format is not depth values in mm" << endl;

        return cv::Mat();
    }

    return cv::Mat();
}

int wmain(int argc, wchar_t *argv[ ], wchar_t *envp[ ] ){

    // Create session
    PXCSmartPtr<PXCSession> session;
    pxcStatus sts=PXCSession_Create(&session);

    if (sts != PXC_STATUS_NO_ERROR) {
        cerr << "Error occured while creating session: " << sts << endl;
         return -1;
    }
/*
    PXCCapture::VideoStream::DataDesc request;

    memset(&request, 0, sizeof(request));

    request.streams[0].format=PXCImage::COLOR_FORMAT_RGB32;
    request.streams[1].format=PXCImage::COLOR_FORMAT_DEPTH;*/

    PXCSmartPtr<PXCCapture> capture;

    sts = session->CreateImpl<PXCCapture>(&capture);

    if (sts != PXC_STATUS_NO_ERROR) {
        cerr << "Error occured while creating capture: " << sts << endl;
         return -1;
    }

    //Print out all devices available

    PXCCapture::DeviceInfo info;
    int firstDevice = -1;

    for(int i=0; i<10; i++){
        sts = capture->QueryDevice(i,&info);
        if (sts != PXC_STATUS_NO_ERROR) {
            cerr << "Error while querying devices: " << sts << endl;
            break;
        }else{
            if (firstDevice<0){
                firstDevice = i;
            }
            cout << "Device " << i << endl
                 << "Name: " << info.name << endl;
        }
    }

    if (firstDevice>=0){
        PXCSmartPtr<PXCCapture::Device> camera;
        sts = capture->CreateDevice(firstDevice,&camera);

        if (sts != PXC_STATUS_NO_ERROR) {
            cerr << "Error while creating a devices: " << sts << endl;
             return -1;
        }

        //check streams available

        PXCCapture::Device::StreamInfo sinfo;
        int depthStreamIdx=-1,colorStreamIdx=-1;

        for(int i=0; i<10; i++){
            sts = camera->QueryStream(i,&sinfo);
            if (sts != PXC_STATUS_NO_ERROR) {
                cerr << "Error while quering streams: " << sts << endl;
                break;
            }else{
                cout << "Stream: " << i << endl
                     << "CUID: " << hex << sinfo.cuid << endl
                     << "image Type: " << sinfo.imageType << endl;
                if (sinfo.imageType == PXCImage::IMAGE_TYPE_COLOR && colorStreamIdx<0){
                    colorStreamIdx = i;
                }
                if (sinfo.imageType == PXCImage::IMAGE_TYPE_DEPTH && depthStreamIdx<0){
                    depthStreamIdx = i;
                }
            }
        }

        //If both streams are found
        if (colorStreamIdx >=0 && depthStreamIdx >=0){
            PXCSmartPtr<PXCCapture::VideoStream> dstream,cstream;

            sts = camera->CreateStream(colorStreamIdx,PXCCapture::VideoStream::CUID,(void **)&cstream);
            if (sts != PXC_STATUS_NO_ERROR){
                cerr << "Error while creating color stream: " << sts << endl;
                 return -1;
            }

            sts = camera->CreateStream(depthStreamIdx,PXCCapture::VideoStream::CUID,(void **)&dstream);
            if (sts != PXC_STATUS_NO_ERROR){
                cerr << "Error while creating depth stream: " << sts << endl;
                 return -1;
            }

            //define image format we want and select it from the list
            int rgbw = 640,rgbh = 480, dw = 320, dh = 240;

            PXCCapture::VideoStream::ProfileInfo pinfo;
            //print stream profiles
            for(int idx=0; ;idx++){
                sts = cstream->QueryProfile(idx,&pinfo);
                if (sts<PXC_STATUS_NO_ERROR) break;

                cout << "Color Stream profile: " << dec << idx << endl
                     << "Image size: " << pinfo.imageInfo.width << ";" << pinfo.imageInfo.height << endl
                     << "Color format: " << hex << pinfo.imageInfo.format << endl;

                if (pinfo.imageInfo.width == rgbw && pinfo.imageInfo.height == rgbh){
                    sts = cstream->SetProfile(&pinfo);
                    if (sts == PXC_STATUS_NO_ERROR){
                        cout << "Color stream profile found" << endl;
                        break;
                    }
                }
            }

            for(int idx=0; ;idx++){
                sts = dstream->QueryProfile(idx,&pinfo);
                if (sts<PXC_STATUS_NO_ERROR) break;

                cout << "Depth Stream: " << dec << idx << endl
                     << "Image info: " << pinfo.imageInfo.width << ";" << pinfo.imageInfo.height << endl
                     << "Depth format: " << hex << pinfo.imageInfo.format << dec << endl;

                if (pinfo.imageInfo.width == dw && pinfo.imageInfo.height == dh){
                    sts = dstream->SetProfile(&pinfo);
                    if (sts == PXC_STATUS_NO_ERROR){
                        cout << "Depth stream profile found" << endl;
                        break;
                    }
                }
            }

            int nstreams=2;
            PXCSmartSPArray sps(nstreams);
            PXCSmartArray<PXCImage> image(nstreams);

            //get image and synchronization pointers
            sts = cstream->ReadStreamAsync(&image[0],&sps[0]);
            if (sts != PXC_STATUS_NO_ERROR){
                cerr << "Error while reading color stream: " << sts << endl;
                return -1;
            }
            sts = dstream->ReadStreamAsync(&image[1],&sps[1]);
            if (sts != PXC_STATUS_NO_ERROR){
                cerr << "Error while reading depth stream: " << sts << endl;
                return -1;
            }

            PXCImage::ImageData cdata,ddata;
            cv::Mat openCVImage;

            //now display(?) the data and print time stamps
            for(;;){
                pxcU32 sidx=0;
                if (sps.SynchronizeEx(&sidx) != PXC_STATUS_NO_ERROR){
                    cout << "Select error" << endl;
                    break;
                }

                //---Query color image---//
                if(sps[0]->Synchronize(0) != PXC_STATUS_NO_ERROR){
                    cout << "Color stream is not ready" << endl;
                }
                else{

                    cout << "Color stream: OK" << endl;

                    sts = image[0]->AcquireAccess(PXCImage::ACCESS_READ,&cdata);
                    if (sts != PXC_STATUS_NO_ERROR){
                        cout << "Failed to acquire access to the color image: " << sts << endl;
                        return -1;
                    }
                    cout << "Color data surface type: " << hex << cdata.type << endl;
                    cout << "Pitches: " << dec << cdata.pitches[0] << ";"
                                               << cdata.pitches[1] << ";"
                                               << cdata.pitches[2] << ";"
                                               << cdata.pitches[3] << endl;

                    openCVImage = convertToOpenCVRGB(cdata,rgbw,rgbh);

                    image[0]->ReleaseAccess(&cdata);

                    cv::imwrite("outputc.png",openCVImage);

                    sps.ReleaseRef(0);

                    sts=cstream->ReadStreamAsync(image.ReleaseRef(0), &sps[0]);
                    if (sts != PXC_STATUS_NO_ERROR){
                        cout << "Error occured while reading an image: " << sts << endl;
                        return -1;
                    }

                }

                //---Query depth image---//
                if(sps[1]->Synchronize(0) != PXC_STATUS_NO_ERROR){
                    cout << "Depth stream is not ready" << endl;
                }
                else{

                    cout << "Depth stream: OK" << endl;

                    sts = image[1]->AcquireAccess(PXCImage::ACCESS_READ,&ddata);
                    if (sts != PXC_STATUS_NO_ERROR){
                        cout << "Failed to acquire access to the depth image: " << sts << endl;
                        return -1;
                    }
                    cout << "Depth data surface type: " << hex << ddata.type << endl;
                    cout << "Pitches: " << dec << ddata.pitches[0] << ";"
                                               << ddata.pitches[1] << ";"
                                               << ddata.pitches[2] << ";"
                                               << ddata.pitches[3] << endl;
                    openCVImage = convertToOpenCVDepth(ddata,dw,dh);

                    image[1]->ReleaseAccess(&ddata);

                    cv::imwrite("outputd.png",openCVImage);

                    sps.ReleaseRef(1);

                    sts=dstream->ReadStreamAsync(image.ReleaseRef(1), &sps[1]);
                    if (sts != PXC_STATUS_NO_ERROR){
                        cout << "Error occured while reading an image: " << sts << endl;
                        return -1;
                    }
                }
            }

            sps.SynchronizeEx();

        }
        else{
            cerr << "Color or depth stream is not found" << endl;
             return -1;
        }

    }

    return 0;
}
